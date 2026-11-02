/**
 * @file    motor.h
 * @brief   Acionamento do motor pelo DRV8871 com o TIM1 (PA8 = IN1, PA9 = IN2).
 *
 * @details O motor fica entre OUT1 e OUT2 e gira sempre para o mesmo lado
 *          (ver MOTOR_SIGN). Logica do DRV8871 (IN1 IN2): 00 = solto (coast),
 *          10 = OUT1 alto, 01 = OUT2 alto, 11 = freio. Com MOTOR_SLOW_DECAY,
 *          a entrada de acionamento fica em nivel alto e a outra pulsa em
 *          nivel baixo durante o ciclo ativo: o motor alterna entre "aciona"
 *          e "freio", o que mantem a corrente lisa (slow decay).
 *
 *          A frequencia do PWM e f_timer / (ARR + 1) e o duty e CCR / (ARR + 1)
 *          (PWM modo 1 do timer avancado).
 *
 * @par Referencias
 *  - DRV8871 datasheet (TI, SLVSCY9B), secoes "PWM Control" e "Bridge
 *    Control": www.ti.com/product/DRV8871/datasheet
 *  - ST, RM0008 (STM32F103), capitulo "Advanced-control timers (TIM1)":
 *    registradores ARR, CCRx e modo PWM 1.
 */
#ifndef MOTOR_H
#define MOTOR_H

#include <stdbool.h>

/**
 * @brief  Ajusta prescaler e periodo do TIM1 para MOTOR_PWM_HZ e liga os dois canais com saida 0 %.
 * @return false se o timer nao partiu.
 */
bool Motor_Init(void);

/**
 * @brief  Define a saida do motor.
 * @param[in] pct  saida em %, limitada a [0, OUT_LIMIT_PCT]. 0 = motor solto.
 */
void Motor_Set(float pct);

#endif /* MOTOR_H */
