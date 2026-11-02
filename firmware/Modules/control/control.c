/**
 * @file    control.c
 * @brief   Implementacao da lei de controle (ver control.h).
 */
#include "control.h"

#include "app_config.h"

void Control_Init(Control_t *c)
{
    /* Saida de MOTOR_IDLE_PCT a OUT_LIMIT_PCT: a helice nunca inverte e, com o
     * controle ativo, nunca para. */
    Pid_Init(&c->pid, MOTOR_IDLE_PCT, OUT_LIMIT_PCT, 0.0f, OUT_LIMIT_PCT);
    c->out_pct = 0.0f;
    c->rate_f  = 0.0f;
    c->active  = false;
}

void Control_Stop(Control_t *c)
{
    c->active  = false;
    c->out_pct = 0.0f;                     /* cortar e sempre imediato */
}

float Control_Step(Control_t *c, const CtrlParams_t *p,
                   float angle_deg, float rate_dps, float dt)
{
    if (!p->armed) {
        Control_Stop(c);
    } else {
        const float step = OUT_SLEW_PCT_S * dt;
        float       req;

        c->pid.anti_windup = p->anti_windup;
        if (!c->active) {
            c->active = true;
            c->rate_f = rate_dps;
            Pid_Reset(&c->pid, OUT_HOVER_PCT);    /* ja parte perto do empuxo de pairar */
        }
        c->rate_f += (dt / (RATE_FILTER_TAU_S + dt)) * (rate_dps - c->rate_f);
        req = Pid_Step(&c->pid, p->kp, p->ki, p->kd,
                       p->setpoint_deg - angle_deg, c->rate_f, dt);
        c->out_pct += Pid_Clamp(req - c->out_pct, -step, step);
    }
    return c->out_pct;
}
