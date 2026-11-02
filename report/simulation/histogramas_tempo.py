#!/usr/bin/env python3
"""Histogramas de latencia (interrupcao -> PWM) e de intervalo entre interrupcoes do TIM2.

Entrada: saida de `python3 tools/dbg_read.py --log > data/raw_readings/final_log3.txt` (linhas hist_*).
Saida:   report/images/results/histogramas_tempo.pdf e os numeros usados no texto.
"""
import os, sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "data", "raw_readings", "final_log3.txt")
lat = per = None
for line in open(src, encoding="utf-8", errors="ignore"):
    if line.startswith("hist_latencia_50us:"):
        lat = np.array([int(x) for x in line.split(":")[1].split()])
    if line.startswith("hist_intervalo_1us_desde_4990:"):
        per = np.array([int(x) for x in line.split(":")[1].split()])
fig, ax = plt.subplots(1, 2, figsize=(8, 3.4))
x = np.arange(len(lat)) * 50
ax[0].bar(x[:20], lat[:20], width=45, align="edge", color="tab:blue")
ax[0].set_xlabel("Latência interrupção → PWM (µs)"); ax[0].set_ylabel("Ciclos"); ax[0].set_yscale("log"); ax[0].grid(alpha=.3)
x2 = 4990 + np.arange(len(per))
ax[1].bar(x2[4:24], per[4:24], width=.9, color="tab:green")
ax[1].set_xlabel("Intervalo entre interrupções (µs)"); ax[1].set_yscale("log"); ax[1].grid(alpha=.3)
fig.tight_layout(); fig.savefig(os.path.join(ROOT, "report", "images", "results", "histogramas_tempo.pdf")); plt.close(fig)
n = lat.sum(); m = per.sum()
print(f"latencia: N={n}; {100*lat[11]/n:.2f}% na caixa 550-600 us, {100*lat[10]/n:.2f}% em 500-550 us, fora delas: {n-lat[10]-lat[11]}")
print(f"intervalo: N={m}; " + ", ".join(f"{4990+i}-{4991+i} us: {per[i]}" for i in range(len(per)) if per[i]))
