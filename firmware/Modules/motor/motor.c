/**
 * @file    motor.c
 * @brief   Implementacao do acionamento do DRV8871 (ver motor.h).
 */
#include "motor.h"

#include <stdint.h>

#include "app_config.h"
#include "tim.h"

bool Motor_Init(void)
{
    RCC_ClkInitTypeDef clk     = { 0 };
    uint32_t           latency = 0U;
    uint32_t           timer_hz;
    uint32_t           ticks;
    uint32_t           prescaler;
    uint32_t           period;
    bool               ok;

    /* Clock que alimenta o TIM1 (APB2; dobra quando o prescaler do APB2 nao e 1) */
    HAL_RCC_GetClockConfig(&clk, &latency);
    timer_hz = HAL_RCC_GetPCLK2Freq();
    if (clk.APB2CLKDivider != RCC_HCLK_DIV1) {
        timer_hz *= 2U;
    }

    /* O CubeMX configurou 20 kHz; aqui a frequencia passa a ser MOTOR_PWM_HZ.
     * O contador tem 16 bits (ate 65536 passos): frequencias baixas pedem prescaler. */
    ticks     = timer_hz / MOTOR_PWM_HZ;
    prescaler = ticks / 65536U;
    period    = (ticks / (prescaler + 1U)) - 1U;
    htim1.Instance->PSC  = prescaler;
    htim1.Instance->ARR  = period;
    htim1.Init.Prescaler = prescaler;
    htim1.Init.Period    = period;
    htim1.Instance->EGR  = TIM_EGR_UG;      /* carrega o prescaler novo agora */
    htim1.Instance->CNT  = 0U;
    htim1.Instance->CCR1 = 0U;
    htim1.Instance->CCR2 = 0U;

    ok = HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) == HAL_OK;
    if (ok) {
        ok = HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) == HAL_OK;
    }
    return ok;
}

void Motor_Set(float pct)
{
    const uint32_t top     = htim1.Instance->ARR + 1U;
    float          limited = pct;
    float          duty_f;
    uint32_t       duty;
    uint32_t       drive;      /* entrada que aciona o motor */
    uint32_t       other;      /* a outra entrada            */

    if (limited < 0.0f) {
        limited = 0.0f;
    }
    if (limited > OUT_LIMIT_PCT) {
        limited = OUT_LIMIT_PCT;
    }
    duty_f = (limited * 0.01f * (float)top) + 0.5f;
    duty   = (uint32_t)duty_f;

    if (duty == 0U) {
        drive = 0U;                         /* IN1=IN2=0: ponte desligada */
        other = 0U;
    } else {
#if MOTOR_SLOW_DECAY
        drive = top;                        /* aciona / freia */
        other = top - duty;
#else
        drive = duty;                       /* aciona / roda livre */
        other = 0U;
#endif
    }

#if MOTOR_SIGN > 0
    htim1.Instance->CCR1 = drive;
    htim1.Instance->CCR2 = other;
#else
    htim1.Instance->CCR1 = other;
    htim1.Instance->CCR2 = drive;
#endif
}
