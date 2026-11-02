/**
 * @file    app.h
 * @brief   Aplicacao do braco de 1 grau de liberdade (1DOF): API para as outras tarefas.
 *
 * @details Blue Pill STM32F103C8 + MPU6050 + DRV8871 + FreeRTOS. Os ajustes
 *          do projeto ficam em app_config.h. Os dados compartilhados (ganhos,
 *          setpoint e telemetria) so sao acessados pelas funcoes abaixo, que
 *          usam um mutex do FreeRTOS.
 *
 * @par Referencias
 *  - FreeRTOS, xSemaphoreGiveFromISR (processamento diferido da interrupcao):
 *    www.freertos.org/a00124.html
 *  - Barry, R. "Mastering the FreeRTOS Real Time Kernel", cap. 6
 *    (gerenciamento de interrupcoes) e cap. 7 (gerenciamento de recursos).
 */
#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>

#include "main.h"
#include "control.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Estado da aplicacao, mostrado pelo LED (ver serial.c). */
typedef enum {
    APP_NO_IMU = 0,           /**< MPU6050 nao responde (motor solto)        */
    APP_CALIBRATING,          /**< medindo o bias do giroscopio (braco parado) */
    APP_RUNNING               /**< controle rodando                          */
} AppState_t;

/** Ganho que um comando da serial quer mudar. */
typedef enum {
    APP_GAIN_KP = 0,          /**< proporcional */
    APP_GAIN_KI,              /**< integral     */
    APP_GAIN_KD               /**< derivativo   */
} AppGain_t;

/** Ultimos valores do controle, para a telemetria. */
typedef struct {
    float angle_deg;          /**< angulo filtrado, graus         */
    float setpoint_deg;       /**< setpoint, graus                */
    float output_pct;         /**< saida aplicada no motor, %     */
} Telemetry_t;

/**
 * @brief  Cria o semaforo, os mutex, a fila e as tarefas.
 * @note   Chamar em freertos.c, na regiao USER CODE BEGIN RTOS_THREADS.
 */
void App_Init(void);

/**
 * @brief  Trata o estouro do TIM2 (a cada 5 ms): libera o semaforo que acorda a tarefa de controle.
 * @param[in] htim  timer que gerou a interrupcao.
 * @note   Chamar em HAL_TIM_PeriodElapsedCallback (main.c, USER CODE BEGIN Callback 1).
 */
void App_TimerISR(const TIM_HandleTypeDef *htim);

/** @brief Estado atual da aplicacao. */
AppState_t App_GetState(void);

/** @brief Copia os parametros do controle (com mutex). @param[out] out destino. */
void       App_GetParams(CtrlParams_t *out);

/** @brief Copia a telemetria (com mutex). @param[out] out destino. */
void       App_GetTelemetry(Telemetry_t *out);

/**
 * @brief  Muda um ganho do PID com o sistema rodando.
 * @param[in] gain   qual ganho.
 * @param[in] value  novo valor, nao negativo.
 * @return false se @p value < 0 (nada muda).
 */
bool       App_SetGain(AppGain_t gain, float value);

/**
 * @brief  Muda o setpoint, em graus, limitado a +-60 graus.
 * @param[in] deg  novo setpoint.
 */
void       App_SetSetpoint(float deg);

/**
 * @brief  Liga ou desliga o motor.
 * @param[in] armed  true = motor liberado.
 */
void       App_SetArmed(bool armed);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
