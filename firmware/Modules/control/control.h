/**
 * @file    control.h
 * @brief   Lei de controle do braco: PID e rampa da saida do motor.
 *
 * @details Sem hardware e sem RTOS, por isso roda tambem no simulador
 *          (tests/sim_control.c). A cada passo:
 *            -# se desarmado, a saida vai a 0 na hora;
 *            -# senao, a velocidade do giroscopio passa por um filtro passa-baixa
 *               de 1a ordem (RATE_FILTER_TAU_S) so para o termo D, para a
 *               vibracao da helice nao virar pulsos na saida;
 *            -# PID sobre (setpoint - angulo), ver pid.h;
 *            -# a saida do motor anda no maximo OUT_SLEW_PCT_S por segundo
 *               (limitador de taxa, "slew rate"), para nao dar degrau de
 *               corrente no DRV8871.
 *
 * @par Referencias
 *  - Limite de corrente e protecao do driver: DRV8871 datasheet (TI,
 *    SLVSCY9B), secao de protecao de sobrecorrente:
 *    www.ti.com/product/DRV8871/datasheet
 *  - Filtro da derivada: Astrom e Hagglund, "PID Controllers", 1995 (filtro
 *    de primeira ordem na parte derivativa para limitar o ganho em alta
 *    frequencia); y[k] = y[k-1] + dt/(tau+dt) * (x[k] - y[k-1]).
 *  - PID: ver pid.h.
 */
#ifndef CONTROL_H
#define CONTROL_H

#include <stdbool.h>
#include <stdint.h>

#include "pid.h"

/** Parametros que podem mudar com o sistema rodando (pela serial). */
typedef struct {
    float kp;                 /**< ganho proporcional, % por grau             */
    float ki;                 /**< ganho integral, % por (grau * s)           */
    float kd;                 /**< ganho derivativo, % por (grau / s)         */
    float setpoint_deg;       /**< angulo desejado, em graus                  */
    bool  armed;              /**< true = motor liberado                      */
    bool  anti_windup;        /**< true = integracao condicional (normal); false = so para ensaio, via ST-Link */
} CtrlParams_t;

/** Estado do controle entre um passo e outro. */
typedef struct {
    Pid_t pid;                /**< PID com os limites da saida                */
    float rate_f;             /**< velocidade filtrada usada no termo D, graus/s */
    float out_pct;            /**< saida aplicada no motor, em %              */
    bool  active;             /**< true = controle atuando neste momento      */
} Control_t;

/**
 * @brief  Prepara o controle: saida limitada a [MOTOR_IDLE_PCT, OUT_LIMIT_PCT], parado.
 * @param[out] c  estado a inicializar.
 */
void Control_Init(Control_t *c);

/**
 * @brief  Um passo de controle.
 * @param[in,out] c          estado do controle.
 * @param[in]     p          parametros atuais (ganhos, setpoint, armado).
 * @param[in]     angle_deg  angulo filtrado, em graus.
 * @param[in]     rate_dps   velocidade angular, em graus/s.
 * @param[in]     dt         periodo de amostragem, em s.
 * @return  saida a aplicar no motor, em %. Desarmado, e 0 na hora.
 */
float Control_Step(Control_t *c, const CtrlParams_t *p,
                   float angle_deg, float rate_dps, float dt);

/**
 * @brief  Para de atuar (saida 0, imediata). Usada quando o sensor some: o
 *         proximo Control_Step recomeca com o integrador em OUT_HOVER_PCT.
 * @param[in,out] c  estado do controle.
 */
void Control_Stop(Control_t *c);

#endif /* CONTROL_H */
