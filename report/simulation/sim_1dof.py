#!/usr/bin/env python3
"""Simulacao do braco 1DOF com a planta identificada na bancada (06/10/2026).

Substitui a simulacao antiga (Drake), que foi perdida. Mesmo modelo do
tests/sim_control.c e de tools/arm_margins.py:

    th'' = G*(empuxo/hover) - G*cos(th) - B*th'      [graus/s2]
    tau*empuxo' = u - empuxo                         [u = saida/100]

G = 6060 graus/s2 (300 graus/s2 por 1 % de PWM x 20,2 % para pairar, medido em 02/11/2026),
tau = 0,058 s (mais 9 ms de atraso puro na leitura), batente de repouso em -38.
O PID e o mesmo de Modules/control/pid.c + control.c (derivada na medida,
filtro da derivada, rampa de saida), com a chave `anti_windup`.

Uso (precisa de numpy e matplotlib):
    python3 sim_1dof.py            # gera ../images/results/*.pdf e imprime metricas
"""
import math, os
from collections import deque
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

DT = 0.005
OUT_LIMIT, IDLE, SLEW, HOVER_INIT = 32.0, 10.0, 300.0, 16.0
KP, KI, KD, TAU_D, SP = 0.06, 0.05, 0.04, 0.005, 0.7
OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "images", "results")


class Arm:
    def __init__(s, G=6060.0, hover=0.202, tau=0.058, B=1.0, rest=-38.0, delay=0.009):
        s.G, s.hover, s.tau, s.B, s.rest = G, hover, tau, B, rest
        s.th, s.w, s.thrust = rest, 0.0, 0.0
        s.buf = deque([(rest, 0.0)] * max(1, round(delay / DT)), maxlen=max(1, round(delay / DT)))  # atraso puro da leitura
        s.top = 60.0       # batente superior suposto: o modelo so vale ate aqui

    def step(s, out_pct, torque_dps2=0.0):
        u = max(out_pct, 0.0) / 100.0
        s.thrust += (u - s.thrust) * DT / s.tau
        acc = s.G / s.hover * s.thrust - s.G * math.cos(math.radians(s.th)) - s.B * s.w + torque_dps2
        s.w += acc * DT
        s.th += s.w * DT
        if s.th < s.rest:
            s.th, s.w = s.rest, max(s.w, 0.0)
        if s.th > s.top:
            s.th, s.w = s.top, min(s.w, 0.0)
        s.buf.append((s.th, s.w))

    def read(s):
        return s.buf[0]


class Controller:
    """PID do firmware (mode="cond"). "none": integrador livre; "clamp": so limita o integrador a [0, OUT_LIMIT]."""
    def __init__(s, mode="cond", kp=KP, ki=KI, kd=KD):
        s.mode, s.kp, s.ki, s.kd = mode, kp, ki, kd
        s.integ, s.rate_f, s.out, s.first = HOVER_INIT, 0.0, 0.0, True

    def step(s, angle, rate, sp=SP):
        if s.first:
            s.rate_f, s.first = rate, False
        s.rate_f += DT / (TAU_D + DT) * (rate - s.rate_f)
        err = sp - angle
        u = s.kp * err + s.integ - s.kd * s.rate_f
        wind_hi = u > OUT_LIMIT and err > 0
        wind_lo = u < IDLE and err < 0
        if s.mode != "cond" or not (wind_hi or wind_lo):
            s.integ += s.ki * err * DT
            if s.mode != "none":
                s.integ = min(max(s.integ, 0.0), OUT_LIMIT)
        req = min(max(u, IDLE), OUT_LIMIT)
        step = SLEW * DT
        s.out += min(max(req - s.out, -step), step)
        return s.out


def run(mode, t_end=30.0, hold=None, gains=(KP, KI, KD), **plant):
    """hold=(t0,t1,ang): a mao segura o braco em `ang` entre t0 e t1."""
    arm, ctl = Arm(**plant), Controller(mode, *gains)
    T, TH, OUT, I = [], [], [], []
    for k in range(int(t_end / DT)):
        t = k * DT
        th, w = arm.read()
        out = ctl.step(th, w)
        arm.step(out)
        if hold and hold[0] <= t < hold[1]:
            arm.th, arm.w = hold[2], 0.0
        T.append(t); TH.append(arm.th); OUT.append(out); I.append(ctl.integ)
    return np.array(T), np.array(TH), np.array(OUT), np.array(I)


def metrics(T, TH, t_from, band=2.0):
    m = T >= t_from
    fora = np.where(np.abs(TH[m] - SP) > band)[0]
    return TH[m].max(), (T[m][fora[-1]] - t_from + DT if len(fora) else 0.0)


MODES = (("cond", "integração condicional (firmware)", "tab:blue"),
         ("clamp", "só limita o integrador", "tab:green"),
         ("none", "sem proteção", "tab:red"))
GANHOS = {"inicial": (1.0, 0.5, 0.15), "final": (KP, KI, KD)}


def fig_windup(nome, gains, hold, titulo):
    fig, ax = plt.subplots(3, 1, figsize=(7.5, 6.5), sharex=True)
    for mode, lab, c in MODES:
        T, TH, OUT, I = run(mode, t_end=hold[1] + 14.0, hold=hold, gains=gains)
        ax[0].plot(T, TH, c, label=lab, lw=1.2); ax[1].plot(T, OUT, c, lw=1.2); ax[2].plot(T, I, c, lw=1.2)
        p, ta = metrics(T, TH, hold[1])
        print(f"{nome:14s} {mode:6s}: integrador max {I.max():7.1f} min {I.min():8.1f} | apos soltar: pico {p:6.1f} graus, a +-2 graus em {ta:5.2f} s")
    for a in ax: a.axvspan(hold[0], hold[1], color="0.9"); a.grid(alpha=.3)
    ax[0].axhline(SP, color="k", ls=":", lw=.8)
    ax[0].set_ylabel("Ângulo (°)"); ax[1].set_ylabel("Saída (%)"); ax[2].set_ylabel("Integrador (%)")
    ax[2].set_xlabel("Tempo (s)"); ax[0].legend(fontsize=8); ax[0].set_title(titulo, fontsize=10)
    ax[2].set_ylim(-40, 60)
    fig.tight_layout(); fig.savefig(os.path.join(OUT_DIR, nome + ".pdf")); plt.close(fig)


def fig_partida():
    fig, ax = plt.subplots(2, 1, figsize=(7.5, 4.6), sharex=True)
    T, TH, OUT, _ = run("cond", t_end=15.0)
    ax[0].plot(T, TH, "tab:blue"); ax[1].plot(T, OUT, "tab:blue")
    print(f"partida (ganhos finais): pico {TH.max():.1f} graus, assenta a +-2 graus em {metrics(T, TH, 0.0)[1]:.2f} s")
    ax[0].axhline(SP, color="k", ls=":", lw=.8); ax[0].set_ylabel("Ângulo (°)"); ax[1].set_ylabel("Saída (%)")
    ax[1].set_xlabel("Tempo (s)")
    for a in ax: a.grid(alpha=.3)
    fig.tight_layout(); fig.savefig(os.path.join(OUT_DIR, "partida.pdf")); plt.close(fig)


if __name__ == "__main__":
    os.makedirs(OUT_DIR, exist_ok=True)
    fig_partida()
    # a mao segura o braco ACIMA do alvo por 5 s (o integrador desce sem a saida mudar) e solta
    fig_windup("windup_ganhos_iniciais", GANHOS["inicial"], (6.0, 11.0, 25.0), "Ganhos 1,0/0,5/0,15: haste retida em +25° por 5 s")
    fig_windup("windup_ganhos_finais", GANHOS["final"], (6.0, 11.0, 25.0), "Ganhos 0,06/0,05/0,04: haste retida em +25° por 5 s")
