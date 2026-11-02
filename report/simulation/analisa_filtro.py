#!/usr/bin/env python3
"""Compara, com dados reais, o angulo SEM filtro (so acelerometro), so giroscopio e o
angulo do firmware (filtro complementar com o acelerometro travado em voo).

Nao precisa regravar o firmware. Coleta (placa ligada, ST-Link conectado), a partir da pasta do projeto:

    python3 firmware/tools/dbg_read.py --set logdiv=1            # 200 Hz, janela de 1,6 s (use 5 para 40 Hz, 8 s)
    ... faca o ensaio (mao levantando o braco com o motor desligado: "m 0"; depois pairando: "m 1") ...
    python3 firmware/tools/dbg_read.py --log > data/raw_readings/motor_ligado.txt

Analise:
    python3 report/simulation/analisa_filtro.py data/raw_readings/motor_desligado.txt data/raw_readings/motor_ligado.txt
Gera report/images/results/filtro_<arquivo>.pdf e imprime desvio padrao e RMS.
"""
import os, re, sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT = os.path.join(os.path.dirname(__file__), "..", "images", "results")


def carrega(path):
    rows, dt = [], None
    for line in open(path, encoding="utf-8", errors="ignore"):
        m = re.search(r"uma a cada (\d+) ms", line)
        if m:
            dt = int(m.group(1)) / 1000.0
        f = line.split()
        if len(f) == 6:
            try:
                rows.append([float(x) for x in f])
            except ValueError:
                pass
    a = np.array(rows)
    return dt, a[:, 0], a[:, 1], a[:, 2], a[:, 3], a[:, 5]   # dt, t, angulo, veloc, ang_acel, saida


def main(paths):
    os.makedirs(OUT, exist_ok=True)
    for p in paths:
        dt, t, ang, rate, acc, out = carrega(p)
        gyro = ang[0] + np.cumsum(rate) * dt
        nome = os.path.splitext(os.path.basename(p))[0]
        fig, ax = plt.subplots(2, 1, figsize=(7.5, 5), sharex=True)
        ax[0].plot(t, acc, color="tab:red", lw=.8, label="sem filtro (só acelerômetro)")
        ax[0].plot(t, gyro, color="tab:green", lw=.9, label="só giroscópio integrado")
        ax[0].plot(t, ang, color="tab:blue", lw=1.3, label="firmware (filtro complementar)")
        ax[0].set_ylabel("Ângulo (°)"); ax[0].legend(fontsize=8); ax[0].grid(alpha=.3)
        ax[1].plot(t, out, color="k"); ax[1].set_ylabel("Saída (%)"); ax[1].set_xlabel("Tempo (s)"); ax[1].grid(alpha=.3)
        fig.tight_layout(); fig.savefig(os.path.join(OUT, "filtro_%s.pdf" % nome)); plt.close(fig)
        d = lambda x: np.sqrt(np.mean((x - ang) ** 2))
        print(f"{nome}: N={len(t)} dt={dt*1000:.0f} ms | desvio padrao: acel {acc.std():.2f}  giro {gyro.std():.2f}  firmware {ang.std():.2f} graus"
              f" | RMS(acel - firmware) {d(acc):.2f}  RMS(giro - firmware) {d(gyro):.2f} graus | saida media {out.mean():.1f} %")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    main(sys.argv[1:])
