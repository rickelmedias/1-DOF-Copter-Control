/**
 * @file    angle_filter.h
 * @brief   Filtro complementar de angulo (giroscopio + acelerometro).
 *
 * @details Sem hardware. O giroscopio e preciso no curto prazo, mas o bias
 *          faz o angulo integrado derivar; o acelerometro nao deriva, mas e
 *          ruidoso e se confunde com vibracao. O filtro soma os dois:
 *
 *   angulo[k] = angulo[k-1] + w*dt + (1 - a) * (acc - (angulo[k-1] + w*dt))
 *   a = tau / (tau + dt)
 *
 * que e o mesmo que a*(angulo + w*dt) + (1 - a)*acc. tau (COMP_TAU_S) e o
 * tempo em que o acelerometro "puxa" o angulo de volta. A diferenca
 * (acc - estimativa) passa por Angle_Wrap180 para nao dar salto ao cruzar
 * +-180 graus.
 *
 * @par Referencias
 *  - Colton, S. "The Balance Filter: A Simple Solution for Integrating
 *    Accelerometer and Gyroscope Measurements for a Balancing Platform",
 *    MIT, 2007: web.mit.edu/scolton/www/filter.pdf
 *  - Angulo pelo acelerometro: NXP AN3461 "Tilt Sensing Using a
 *    Three-Axis Accelerometer", Rev. 6, 2013 (equacoes de roll e pitch):
 *    www.nxp.jp/docs/en/application-note/AN3461.pdf
 */
#ifndef ANGLE_FILTER_H
#define ANGLE_FILTER_H

/**
 * @brief  Leva um angulo para o intervalo [-180, +180).
 * @param[in] a  angulo, em graus.
 * @return  angulo equivalente dentro do intervalo.
 */
float Angle_Wrap180(float a);

/**
 * @brief  Um passo do filtro complementar.
 * @param[in] angle      estimativa anterior, em graus.
 * @param[in] acc_angle  angulo calculado pelo acelerometro, em graus.
 * @param[in] rate       velocidade do giroscopio SEM o bias, em graus/s.
 * @param[in] dt         periodo de amostragem, em s.
 * @param[in] tau_s      constante de tempo do filtro, em s (menor = confia mais no
 *                       acelerometro). tau_s <= 0 ignora o acelerometro (so giroscopio).
 * @param[in] gate_deg   se > 0, ignora o acelerometro neste passo quando ele diferir da
 *                       estimativa por mais que gate_deg (descarta leituras absurdas
 *                       causadas pela vibracao da helice). 0 = sem limite.
 * @return  nova estimativa do angulo, em graus, em [-180, +180).
 */
float Angle_FilterStep(float angle, float acc_angle, float rate, float dt,
                       float tau_s, float gate_deg);

#endif /* ANGLE_FILTER_H */
