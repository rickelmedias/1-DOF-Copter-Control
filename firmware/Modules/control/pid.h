/**
 * @file    pid.h
 * @brief   Controlador PID discreto com derivada na medida e anti-windup.
 *
 * @details Implementacao propria, sem biblioteca. Sem hardware e sem RTOS.
 *
 * Forma usada, com e = setpoint - medida e amostragem fixa dt:
 *
 *   u[k] = Kp*e[k] + I[k] - Kd*dy[k]/dt
 *   I[k] = I[k-1] + Ki*e[k]*dt
 *
 * - Derivada na medida: como o setpoint e constante entre degraus,
 *   de/dt = -dy/dt. Usar so a medida evita o "tranco" (derivative kick)
 *   quando o setpoint muda. Aqui dy/dt e a velocidade do giroscopio.
 * - Anti-windup por integracao condicional (desligavel com anti_windup = false, so para
 *   comparar em ensaio; o integrador continua limitado a [i_min, i_max]): nao integra enquanto a saida
 *   esta saturada e o erro a empurra ainda mais para dentro da saturacao.
 *
 * @par Referencias
 *  - Astrom, K. J. e Hagglund, T. "PID Controllers: Theory, Design and
 *    Tuning", 2a ed., ISA, 1995 (derivada na medida e anti-windup).
 *  - Astrom, K. J. e Murray, R. M. "Feedback Systems", cap. 10 (secao
 *    "Integrator Windup"): www.cds.caltech.edu/~murray/amwiki/PID_Control.html
 *  - Beauregard, B. "Improving the Beginner's PID" (derivative kick e reset
 *    windup): www.datsi.fi.upm.es/docencia/DMC/guia_pid/improving-the-beginners-pid-derivative-kick
 */
#ifndef PID_H
#define PID_H

#include <stdbool.h>

/** Estado e limites de um PID. */
typedef struct {
    float integ;              /**< termo integral acumulado, na unidade da saida */
    float out_min;            /**< limite inferior da saida                      */
    float out_max;            /**< limite superior da saida                      */
    float i_min;              /**< limite inferior do termo integral             */
    float i_max;              /**< limite superior do termo integral             */
    bool  anti_windup;        /**< true = integracao condicional; false = so limita o integrador (para ensaio) */
} Pid_t;

/**
 * @brief  Define os limites e zera o integrador.
 * @param[out] pid      PID a inicializar.
 * @param[in]  out_min  menor saida permitida.
 * @param[in]  out_max  maior saida permitida.
 * @param[in]  i_min    menor valor do termo integral.
 * @param[in]  i_max    maior valor do termo integral.
 */
void  Pid_Init(Pid_t *pid, float out_min, float out_max, float i_min, float i_max);

/**
 * @brief  Recomeca o integrador a partir de um valor (usado ao rearmar o controle).
 * @param[in,out] pid    PID a reiniciar.
 * @param[in]     integ  valor inicial do termo integral, limitado a [i_min, i_max].
 */
void  Pid_Reset(Pid_t *pid, float integ);

/**
 * @brief  Calcula um passo do PID.
 * @param[in,out] pid  estado do PID (o integrador e atualizado).
 * @param[in] kp    ganho proporcional, em saida por unidade de erro.
 * @param[in] ki    ganho integral, em saida por (erro * s).
 * @param[in] kd    ganho derivativo, em saida por (unidade da medida / s).
 * @param[in] err   erro = setpoint - medida.
 * @param[in] rate  derivada da MEDIDA (aqui, velocidade do giroscopio).
 * @param[in] dt    periodo de amostragem, em s.
 * @return  saida ja limitada a [out_min, out_max].
 */
float Pid_Step(Pid_t *pid, float kp, float ki, float kd,
               float err, float rate, float dt);

/**
 * @brief  Limita um valor a um intervalo.
 * @param[in] v   valor.
 * @param[in] lo  limite inferior.
 * @param[in] hi  limite superior.
 * @return  v limitado a [lo, hi].
 */
float Pid_Clamp(float v, float lo, float hi);

#endif /* PID_H */
