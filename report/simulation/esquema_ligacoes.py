#!/usr/bin/env python3
"""Diagrama de ligacoes do prototipo (fiacao por pino). Gera images/hardware/esquema_ligacoes.pdf.

Baseado em: topicos2.ioc (pinos do STM32), foto do modulo DRV8871 (OUT2 OUT1 IN2 IN1 VM GND) e na
descricao da montagem (fonte 12 V, BEC, chave, indutor, motor; sem capacitor). Todos os GND sao comuns.
"""
import os
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Circle, FancyBboxPatch

OUT = os.path.join(os.path.dirname(__file__), "..", "images", "hardware", "esquema_ligacoes.pdf")
C12, C5, C33, CG, CS, CI = "#c0392b", "#e67e22", "#8e44ad", "#222222", "#2471a3", "#1e8449"

fig, ax = plt.subplots(figsize=(12, 7.4))
ax.set_xlim(0, 122); ax.set_ylim(2, 78); ax.axis("off")


def box(x, y, w, h, title):
    ax.add_patch(FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.2,rounding_size=0.8", fc="#f4f6f7", ec="#555", lw=1.2, zorder=1))
    ax.text(x + w / 2, y + h + 1.0, title, ha="center", va="bottom", fontsize=8.5, fontweight="bold")


def pin(x, y, label, side, color="k", fs=7.2):
    ax.plot([x], [y], "o", ms=3.8, color=color, zorder=5)
    ax.text(x + (1.1 if side == "in" else -1.1), y, label, ha="left" if side == "in" else "right", va="center", fontsize=fs, zorder=5)
    return (x, y)


def wire(pts, color, lw=1.7, ls="-"):
    xs, ys = zip(*pts)
    ax.plot(xs, ys, color=color, lw=lw, ls=ls, zorder=3, solid_capstyle="round")


def dot(p, color):
    ax.plot([p[0]], [p[1]], "o", ms=3.2, color=color, zorder=6)


def gnd(p, dx=0.0, dy=-1.8):
    """Fio curto ate o simbolo de terra."""
    x, y = p
    wire([(x, y), (x + dx, y), (x + dx, y + dy)], CG, lw=1.4)
    for k, w in enumerate((1.6, 1.0, 0.45)):
        ax.plot([x + dx - w, x + dx + w], [y + dy - 0.5 * k, y + dy - 0.5 * k], color=CG, lw=1.4, zorder=3)


# ---------------- fonte 12 V e BEC
box(2, 54, 12, 11, "Fonte 12 V")
fp = pin(14, 62, "+", "out", C12, 9); fn = pin(14, 57, "−", "out", CG, 9)
box(21, 54, 14, 11, "BEC 12 → 5 V")
bip = pin(21, 62, "IN+", "in", C12); bin_ = pin(21, 57, "IN−", "in", CG)
bop = pin(35, 62, "OUT+", "out", C5); bon = pin(35, 57, "OUT−", "out", CG)
wire([fp, bip], C12); wire([fn, bin_], CG)
gnd(fn, dx=3.5); gnd(bon, dx=2.2, dy=-1.8)

# ---------------- chave (2 posicoes, 1 terminal livre)
box(40, 55, 10, 10, "Chave")
s1 = pin(40, 62, "1", "in", C5); s2 = pin(50, 62, "2", "out", C5)
ax.plot([41.2, 48.8], [62, 64.2], color="k", lw=1.8, zorder=4)
ax.plot([45], [57.5], "o", ms=3.8, mfc="white", mec="#888", zorder=5)
ax.text(45, 56.1, "3: livre", ha="center", fontsize=6.3, color="#666")
ax.text(45, 59.8, "↑ ON   ↓ OFF", ha="center", fontsize=6.3, color="#555")
wire([bop, s1], C5)

# ---------------- Blue Pill
box(56, 8, 15, 47, "Blue Pill\n(STM32F103)")
bp5 = pin(56, 50, "5V", "in", C5); bpg = pin(56, 45, "GND", "in", CG); bp33 = pin(56, 40, "3V3", "in", C33)
bpb6 = pin(56, 30, "PB6", "in", CI); bpb7 = pin(56, 25, "PB7", "in", CI)
bpa8 = pin(71, 50, "PA8", "out", CS); bpa9 = pin(71, 45, "PA9", "out", CS)
bpd = pin(71, 15, "PA13", "out", "#777"); bpc = pin(71, 11, "PA14", "out", "#777")
ax.text(63.5, 18.4, "PA2/PA3: serial (opcional)\nPC13: LED", ha="center", fontsize=6.2, color="#666")
wire([s2, (52.5, 62), (52.5, 50), bp5], C5)
gnd(bpg, dx=-2.5)

# ---------------- MPU6050
box(2, 12, 17, 26, "MPU6050 (GY-521)")
mv = pin(19, 33, "VCC", "out", C33); mg = pin(19, 28, "GND", "out", CG); ms = pin(19, 23, "SCL", "out", CI); md = pin(19, 18, "SDA", "out", CI)
ax.text(10.5, 14.2, "AD0 em nível baixo (0x68)\nINT não usado\nVCC ligado ao 3V3", ha="center", fontsize=6.3, color="#555")
gnd(mg, dx=2.5)
wire([mv, (45, 33), (45, 40), bp33], C33)
wire([ms, (41, 23), (41, 30), bpb6], CI)
wire([md, (43, 18), (43, 25), bpb7], CI)

# ---------------- DRV8871
box(86, 34, 17, 24, "Driver DRV8871\n(módulo)")
dvm = pin(86, 55, "VM", "in", C12); di1 = pin(86, 50, "IN1", "in", CS); di2 = pin(86, 45, "IN2", "in", CS); dg = pin(86, 40, "GND", "in", CG)
do1 = pin(103, 50, "OUT1", "out", "k"); do2 = pin(103, 45, "OUT2", "out", "k")
wire([bpa8, di1], CS); wire([bpa9, di2], CS)
wire([fp, (17.5, 62), (17.5, 73), (83, 73), (83, 55), dvm], C12); dot((17.5, 62), C12)
gnd(dg, dx=-3.0)
ax.text(81.5, 52.5, "sem capacitor\nem VM", ha="right", va="center", fontsize=6.3, color="#777")

# ---------------- indutor + motor
mot = (93, 18)
ax.add_patch(Circle(mot, 4.4, fc="#fdfefe", ec="k", lw=1.5, zorder=4)); ax.text(mot[0], mot[1], "M", ha="center", va="center", fontsize=12, fontweight="bold", zorder=5)
ax.text(mot[0], mot[1] - 6.8, "Motor 0720 + hélice", ha="center", fontsize=7)
# OUT1 -> indutor -> motor ; OUT2 -> motor
wire([do1, (107, 50), (107, 40)], "k")
cy = np.linspace(40, 31, 240); cx = 107 + 1.5 * np.abs(np.sin(np.linspace(0, 4 * np.pi, 240)))
ax.plot(cx, cy, color="k", lw=1.7, zorder=4)
wire([(107, 31), (107, 18), (mot[0] + 4.4, 18)], "k")
wire([do2, (105, 45), (105, 26), (93, 26), (93, 22.4)], "k")
ax.text(109.2, 35.5, "L = 220 µH, 3 A\n(em série com\no fio de OUT1)", ha="left", va="center", fontsize=6.6)

# ---------------- ST-Link
ax.add_patch(Rectangle((76, 8), 9.5, 10, fc="#fdfefe", ec="#999", ls="--", lw=1, zorder=1))
ax.text(80.7, 13, "ST-Link V2\nSWDIO, SWCLK", ha="center", va="center", fontsize=6.4)
wire([bpd, (76, 15)], "#999", ls="--"); wire([bpc, (73.5, 11), (73.5, 11), (76, 11)], "#999", ls="--")
ax.text(80.7, 6.2, "só para gravar e ler diagnóstico", ha="center", fontsize=6.2, color="#666")

# ---------------- legenda e titulo
for i, (t, c) in enumerate([("12 V", C12), ("5 V", C5), ("3,3 V", C33), ("GND", CG), ("PWM", CS), ("I²C", CI)]):
    ax.plot([44 + i * 11.5, 46.5 + i * 11.5], [76.5, 76.5], color=c, lw=3); ax.text(47 + i * 11.5, 76.5, t, fontsize=6.6, va="center")
ax.text(2, 76.5, "Diagrama de ligações do protótipo", fontsize=11, fontweight="bold")
fig.savefig(OUT, bbox_inches="tight"); print("gerado")
