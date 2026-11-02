/**
 * @file    pid.c
 * @brief   Implementacao do PID (ver pid.h para a formula e as referencias).
 */
#include "pid.h"

#include <stdbool.h>

float Pid_Clamp(float v, float lo, float hi)
{
    float r = v;

    if (v < lo) {
        r = lo;
    } else if (v > hi) {
        r = hi;
    } else {
        /* ja esta dentro da faixa */
    }
    return r;
}

void Pid_Init(Pid_t *pid, float out_min, float out_max, float i_min, float i_max)
{
    pid->integ   = 0.0f;
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->i_min   = i_min;
    pid->i_max   = i_max;
    pid->anti_windup = true;
}

void Pid_Reset(Pid_t *pid, float integ)
{
    pid->integ = Pid_Clamp(integ, pid->i_min, pid->i_max);
}

float Pid_Step(Pid_t *pid, float kp, float ki, float kd,
               float err, float rate, float dt)
{
    /* D usa -rate (derivada da medida): nao da "tranco" quando o setpoint muda */
    const float u = ((kp * err) + pid->integ) - (kd * rate);

    /* Anti-windup: nao integra quando a saida esta saturada e o erro a
     * empurra ainda mais para dentro da saturacao */
    const bool wind_hi = (u > pid->out_max) && (err > 0.0f);
    const bool wind_lo = (u < pid->out_min) && (err < 0.0f);

    if ((!pid->anti_windup) || ((!wind_hi) && (!wind_lo))) {
        pid->integ = Pid_Clamp(pid->integ + (ki * err * dt), pid->i_min, pid->i_max);
    }
    return Pid_Clamp(u, pid->out_min, pid->out_max);
}
