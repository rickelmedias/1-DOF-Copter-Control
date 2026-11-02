#!/usr/bin/env python3
"""Compara os 5 testes de refino (PID e anti-windup) com o mesmo N = 1200 amostras (60 s a 20 Hz).

Entrada: data/tests/T*.csv (firmware/tools/captura_longa.py). Saida: report/images/results/refino_*.pdf,
data/tests/metricas.csv e a tabela impressa para o texto.
"""
import csv, glob, os
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
OUT = os.path.join(ROOT, "report", "images", "results")
SP = 0.7
LABEL = {"T1_atual": "T1 atual 0,06/0,05/0,04", "T2_aw_desligado": "T2 sem anti-windup",
         "T3_ki008": "T3 Ki 0,08", "T4_kp008_ki008": "T4 Kp 0,08 Ki 0,08", "T5_ki009": "T5 Ki 0,09"}


def load(path):
    with open(path) as f:
        rows = list(csv.DictReader(f))
    g = lambda k: np.array([float(r[k]) for r in rows])
    return {k: g(k) for k in ("t_s", "angulo_graus", "velocidade_dps", "ang_acel_graus", "saida_pct")}, rows[0]


def metricas(d):
    a, o, r = d["angulo_graus"], d["saida_pct"], d["velocidade_dps"]
    e = a - SP
    return dict(N=len(a), media=a.mean(), desvio=a.std(), rms=np.sqrt(np.mean(e ** 2)), maxabs=np.abs(e).max(),
                pp=a.max() - a.min(), saida_media=o.mean(), saida_desvio=o.std(),
                atividade=np.mean(np.abs(np.diff(o))), vel_rms=np.sqrt(np.mean(r ** 2)))


def main():
    os.makedirs(OUT, exist_ok=True)
    files = sorted(glob.glob(os.path.join(ROOT, "data", "tests", "T*.csv")))
    data = {}
    for f in files:
        name = os.path.splitext(os.path.basename(f))[0]
        d, r0 = load(f)
        data[name] = (d, r0, metricas(d))
    names = list(data)
    # --- tabela
    print(f"{'teste':18s} {'N':>5s} {'media':>6s} {'desv':>5s} {'RMS':>5s} {'max|e|':>6s} {'p-p':>5s} {'saida':>6s} {'dp_saida':>8s} {'ativ.':>6s} {'vel_rms':>7s}")
    with open(os.path.join(ROOT, "data", "tests", "metricas.csv"), "w", newline="") as f:
        w = csv.writer(f); w.writerow(["teste", "kp", "ki", "kd", "aw", "N", "media", "desvio", "rms_erro", "max_abs_erro", "pico_a_pico", "saida_media", "saida_desvio", "atividade", "vel_rms"])
        for n in names:
            d, r0, m = data[n]
            print(f"{n:18s} {m['N']:5d} {m['media']:6.2f} {m['desvio']:5.2f} {m['rms']:5.2f} {m['maxabs']:6.2f} {m['pp']:5.2f} {m['saida_media']:6.2f} {m['saida_desvio']:8.3f} {m['atividade']:6.3f} {m['vel_rms']:7.2f}")
            w.writerow([n, r0["kp"], r0["ki"], r0["kd"], r0["aw"], m["N"], f"{m['media']:.3f}", f"{m['desvio']:.3f}", f"{m['rms']:.3f}", f"{m['maxabs']:.3f}", f"{m['pp']:.3f}", f"{m['saida_media']:.3f}", f"{m['saida_desvio']:.4f}", f"{m['atividade']:.4f}", f"{m['vel_rms']:.3f}"])
    cores = plt.cm.tab10(np.arange(len(names)))
    # --- 1) sobreposicao das curvas
    fig, ax = plt.subplots(2, 1, figsize=(8, 5.2), sharex=True)
    for c, n in zip(cores, names):
        d = data[n][0]
        ax[0].plot(d["t_s"], d["angulo_graus"], color=c, lw=.9, label=LABEL.get(n, n))
        ax[1].plot(d["t_s"], d["saida_pct"], color=c, lw=.9)
    ax[0].axhline(SP, color="k", ls=":", lw=.8); ax[0].set_ylabel("Ângulo (°)"); ax[1].set_ylabel("Saída (%)"); ax[1].set_xlabel("Tempo após estabilizar (s)")
    ax[0].legend(fontsize=7, ncol=2); [a.grid(alpha=.3) for a in ax]
    fig.tight_layout(); fig.savefig(os.path.join(OUT, "refino_curvas.pdf")); plt.close(fig)
    # --- 2) um painel por teste
    fig, axs = plt.subplots(len(names), 1, figsize=(8, 1.7 * len(names)), sharex=True, sharey=True)
    for ax_, c, n in zip(axs, cores, names):
        d, _, m = data[n]
        ax_.plot(d["t_s"], d["angulo_graus"], color=c, lw=.9)
        ax_.axhline(SP, color="k", ls=":", lw=.7)
        ax_.set_ylabel("°", rotation=0); ax_.set_title(f"{LABEL.get(n, n)}: RMS {m['rms']:.2f}°, desvio {m['desvio']:.2f}°, pico a pico {m['pp']:.1f}°", fontsize=8, loc="left")
        ax_.grid(alpha=.3)
    axs[-1].set_xlabel("Tempo após estabilizar (s)")
    fig.tight_layout(); fig.savefig(os.path.join(OUT, "refino_paineis.pdf")); plt.close(fig)
    # --- 3) histogramas do erro
    fig, ax = plt.subplots(figsize=(7, 3.6))
    allv = np.concatenate([data[n][0]["angulo_graus"] for n in names])
    bins = np.linspace(allv.min() - .1, allv.max() + .1, 40)
    for c, n in zip(cores, names):
        ax.hist(data[n][0]["angulo_graus"], bins=bins, histtype="step", color=c, lw=1.2, label=LABEL.get(n, n))
    ax.axvline(SP, color="k", ls=":", lw=.8); ax.set_xlabel("Ângulo (°)"); ax.set_ylabel("Amostras"); ax.legend(fontsize=7); ax.grid(alpha=.3)
    fig.tight_layout(); fig.savefig(os.path.join(OUT, "refino_histograma.pdf")); plt.close(fig)
    # --- 4) metricas
    fig, ax = plt.subplots(1, 3, figsize=(9, 3.2))
    x = np.arange(len(names)); tick = [n.split("_")[0] for n in names]
    for a_, key, tit in zip(ax, ("rms", "pp", "atividade"), ("RMS do erro (°)", "Pico a pico (°)", "Atividade da saída (% por amostra)")):
        a_.bar(x, [data[n][2][key] for n in names], color=cores); a_.set_xticks(x); a_.set_xticklabels(tick); a_.set_title(tit, fontsize=9); a_.grid(alpha=.3, axis="y")
    fig.tight_layout(); fig.savefig(os.path.join(OUT, "refino_metricas.pdf")); plt.close(fig)


if __name__ == "__main__":
    main()
