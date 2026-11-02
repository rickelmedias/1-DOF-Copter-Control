/**
 * @file    angle_filter.c
 * @brief   Implementacao do filtro complementar (ver angle_filter.h).
 */
#include "angle_filter.h"

#include <math.h>

#include "app_config.h"

float Angle_Wrap180(float a)
{
    float r = fmodf(a + 180.0f, 360.0f);   /* resto entre -360 e +360 */

    if (r < 0.0f) {
        r += 360.0f;
    }
    return r - 180.0f;
}

/* Giroscopio no curto prazo, acelerometro no longo prazo.
 * A correcao usa o erro com wrap, para nao dar salto ao cruzar +-180 graus. */
float Angle_FilterStep(float angle, float acc_angle, float rate, float dt,
                       float tau_s, float gate_deg)
{
    float est = angle + (rate * dt);

    if (tau_s > 0.0f) {
        const float alpha = tau_s / (tau_s + dt);
        const float err   = Angle_Wrap180(acc_angle - est);

        if ((gate_deg <= 0.0f) || (fabsf(err) <= gate_deg)) {
            est += (1.0f - alpha) * err;
        }
    }
    return Angle_Wrap180(est);
}
