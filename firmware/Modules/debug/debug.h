/**
 * @file    debug.h
 * @brief   Diagnostico guardado na RAM, para ler pelo ST-Link sem serial.
 *
 * @details Ligado por DEBUG_ENABLE (app_config.h). Tudo fica na variavel
 *          global g_debug, lida por tools/dbg_read.py pelo ST-Link (a RAM e
 *          lida em "hot plug", sem parar o processador); quem mudar
 *          DebugData_t muda o formato la tambem. Roda so no contexto das
 *          tarefas: sem mutex, sem espera, sem interrupcao.
 *
 * @par Referencias
 *  Implementacao propria. Leitura da RAM: STM32CubeProgrammer, manual do
 *  usuario UM2237 (opcoes -r8 e mode=HOTPLUG).
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#include "app_config.h"

/** @name Indice de cada tarefa em DebugData_t::stack_free */
/**@{*/
#define DEBUG_TASK_CONTROL  0U
#define DEBUG_TASK_CMD      1U
#define DEBUG_TASK_TELEM    2U
#define DEBUG_TASK_COUNT    3U
/**@}*/

/** Caixas dos histogramas de tempo em DebugData_t. */
#define DEBUG_HIST_N        32U

/** Contadores de eventos. */
typedef enum {
    DEBUG_READ_FAIL = 0,      /* leituras do MPU6050 que falharam            */
    DEBUG_BUS_RECOVER,        /* reinicios do I2C                            */
    DEBUG_UART_ERROR,         /* erros de recepcao na USART2                 */
    DEBUG_COUNT_N
} DebugCount_t;

#if DEBUG_ENABLE

/** Dados lidos pelo ST-Link. Os tipos tem tamanho fixo e sem preenchimento (o script le por offset). */
typedef struct {
    uint32_t count[DEBUG_COUNT_N];
    uint32_t samples;         /* amostras ja vistas por Debug_Sample          */
    float    angle_max;       /* graus                                        */
    float    angle_min;
    float    rate_max;        /* graus/s, em modulo                           */
    uint32_t stack_free[DEBUG_TASK_COUNT];   /* menor folga de pilha, em palavras */
    uint16_t log_div;         /* decimacao do registro: comeca em DEBUG_LOG_DIV, ajustavel pelo ST-Link */
    uint16_t log_size;        /* copia de DEBUG_LOG_N                         */
    uint32_t log_n;           /* total de amostras gravadas (registro circular) */
    /* angulo x10, velocidade x10, aceleracao X e Z em milesimos de g, saida x10 */
    int16_t  log[DEBUG_LOG_N][5];
    /* Tempos medidos com o contador de ciclos do DWT (72 MHz: 1 ciclo = 13,9 ns).
     * Ficam DEPOIS do registro para nao mudar o deslocamento dos campos acima. */
    uint32_t timing_ticks;    /* interrupcoes do TIM2 contadas                            */
    uint32_t timing_takes;    /* vezes em que a tarefa de controle foi acordada           */
    uint32_t period_min;      /* menor intervalo entre duas interrupcoes do TIM2, ciclos  */
    uint32_t period_max;      /* maior intervalo, ciclos                                  */
    uint32_t wake_max;        /* maior atraso interrupcao -> tarefa acordada, ciclos      */
    uint32_t exec_max;        /* maior tempo tarefa acordada -> PWM escrito, ciclos       */
    uint32_t latency_max;     /* maior atraso interrupcao -> PWM escrito, ciclos          */
    uint32_t latency_sum_us;  /* soma das latencias, em microssegundos                    */
    uint32_t latency_n;       /* ciclos somados                                           */
    uint32_t deadline_miss;   /* ciclos com latencia maior que um periodo de controle     */
    uint32_t heap_free;       /* bytes livres no heap do FreeRTOS na ultima medida        */
    uint32_t heap_min_free;   /* menor folga de heap ja vista, bytes                      */
    float    gyro_bias_dps;   /* bias do giroscopio medido na calibracao, graus/s         */
    float    start_angle_deg; /* angulo inicial medido na calibracao, graus               */
    float    cutoff_angle_deg;/* angulo que causou o ultimo corte por angulo, graus       */
    uint32_t cutoff_count;    /* cortes por angulo desde o reset                          */
    uint32_t hist_latency[DEBUG_HIST_N];  /* latencia interrupcao -> PWM, caixas de 50 us (ultima = maior) */
    uint32_t hist_period[DEBUG_HIST_N];   /* intervalo entre interrupcoes, caixas de 1 us a partir de 4990 us */
} DebugData_t;

extern volatile DebugData_t g_debug;

/** @brief Soma 1 ao contador @p what. */
void Debug_Count(DebugCount_t what);

/**
 * @brief  Chamada pela tarefa de controle a cada amostra valida: guarda os
 *         extremos de angulo e de velocidade e, 1 a cada DEBUG_LOG_DIV
 *         chamadas, uma linha do registro circular.
 * @param[in] angle_deg      angulo filtrado, graus.
 * @param[in] rate_dps       velocidade angular sem bias, graus/s.
 * @param[in] acc_x_g        componente X da aceleracao (a que entra no angulo), em g.
 * @param[in] acc_z_g        componente Z da aceleracao, em g.
 * @param[in] out_pct        saida do motor, %.
 */
void Debug_Sample(float angle_deg, float rate_dps, float acc_x_g, float acc_z_g, float out_pct);

/**
 * @brief  Guarda a folga minima de pilha da tarefa que chamou e a folga do heap.
 * @param[in] idx  DEBUG_TASK_CONTROL, DEBUG_TASK_CMD ou DEBUG_TASK_TELEM.
 */
void Debug_StackMark(uint32_t idx);

/** @brief Liga o contador de ciclos do DWT (Cortex-M3). Chamar uma vez, antes de iniciar o TIM2. */
void Debug_TimingInit(void);

/** @brief Na ISR do TIM2: guarda o instante da interrupcao e mede o intervalo desde a anterior. */
void Debug_TimingTick(void);

/** @brief Na tarefa de controle, logo depois de o semaforo do TIM2 chegar: marca o inicio do ciclo. */
void Debug_TimingBegin(void);

/** @brief Na tarefa de controle, logo depois de escrever o PWM: fecha as medidas do ciclo. */
void Debug_TimingEnd(void);

/** @brief Guarda o bias do giroscopio e o angulo inicial medidos na calibracao. */
void Debug_Calibration(float gyro_bias_dps, float start_angle_deg);

/** @brief Conta um corte por angulo e guarda o angulo que o causou. */
void Debug_CutoffTrip(float angle_deg);

#else   /* diagnostico desligado: chamadas vazias */

static inline void Debug_Count(DebugCount_t what)
{
    (void)what;
}

static inline void Debug_Sample(float angle_deg, float rate_dps, float acc_x_g, float acc_z_g, float out_pct)
{
    (void)angle_deg;
    (void)rate_dps;
    (void)acc_x_g;
    (void)acc_z_g;
    (void)out_pct;
}

static inline void Debug_StackMark(uint32_t idx)
{
    (void)idx;
}

static inline void Debug_TimingInit(void)
{
}

static inline void Debug_TimingTick(void)
{
}

static inline void Debug_TimingBegin(void)
{
}

static inline void Debug_TimingEnd(void)
{
}

static inline void Debug_Calibration(float gyro_bias_dps, float start_angle_deg)
{
    (void)gyro_bias_dps;
    (void)start_angle_deg;
}

static inline void Debug_CutoffTrip(float angle_deg)
{
    (void)angle_deg;
}

#endif /* DEBUG_ENABLE */

#endif /* DEBUG_H */
