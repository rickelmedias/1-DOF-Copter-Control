#!/usr/bin/env python3
"""Analise dos degraus de setpoint (+10, -20, volta) para os 5 conjuntos de ganhos.

Entrada: data/steps/D*.csv e *_eventos.txt (firmware/tools/captura_degraus.py).
Saida: report/images/results/refino_degraus*.pdf, data/steps/metricas_degraus.csv e a tabela para o texto.
"""
import csv, glob, os, re
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
OUT = os.path.join(ROOT, "report", "images", "results")
LABEL = {"D1_atual": "T1 atual 0,06/0,05/0,04", "D2_aw_desligado": "T2 sem anti-windup", "D3_ki008": "T3 Ki 0,08",
         "D4_kp008_ki008": "T4 Kp 0,08 Ki 0,08", "D5_ki009": "T5 Ki 0,09"}
STEPS = [("+10", 10.7), ("-20", -19.3), ("volta", 0.7)]
W = 12.0   # janela de analise apos cada degrau, em s


def load(name, pasta="steps"):
    rows = list(csv.DictReader(open(os.path.join(ROOT, "data", pasta, name + ".csv"))))
    t = np.array([float(r["t_s"]) for r in rows]); a = np.array([float(r["angulo_graus"]) for r in rows])
    o = np.array([float(r["saida_pct"]) for r in rows])
    t = t - t[0]
    ev = [float(re.search(r"instante_relogio_s=([\d.]+)", l).group(1)) for l in open(os.path.join(ROOT, "data", pasta, name + "_eventos.txt"))]
    return t, a, o, ev


def analisa(name, pasta="steps"):
    t, a, o, ev = load(name, pasta)
    base = a[(t > 3) & (t < 9)].mean()
    on1 = t[np.argmax((t > 10) & (a > base + 5.0))] - 1.0
    onsets = [on1 + (e - ev[0]) for e in ev]
    res = {"base": base, "onsets": onsets}
    dt = np.mean(np.diff(t))
    for k, (nome, alvo) in enumerate(STEPS):
        m = (t >= onsets[k]) & (t < onsets[k] + W)
        tt, aa = t[m] - onsets[k], a[m]
        a0 = a[(t >= onsets[k] - 1.0) & (t < onsets[k])].mean()
        amp = alvo - a0
        frac = (aa - a0) / amp
        t90 = tt[np.argmax(frac >= 0.9)] if (frac >= 0.9).any() else np.nan
        pico = (frac.max() - 1.0) * 100.0
        fin = aa[-int(3 / dt):].mean() - alvo
        iae = np.sum(np.abs(aa - alvo)) * dt
        res[nome] = dict(amp=amp, t90=t90, sobre=pico, erro_final=fin, iae=iae)
    return t, a, o, onsets, res


def main():
    nomes = sorted(os.path.basename(f)[:-4] for f in glob.glob(os.path.join(ROOT, "data", "steps", "D*.csv")) if "_eventos" not in f)
    cores = plt.cm.tab10(np.arange(len(nomes)))
    todos = {n: analisa(n) for n in nomes}
    print(f"{'teste':16s} | {'degrau +10':^34s} | {'degrau -20':^34s} | {'volta a 0,7':^34s}")
    print(f"{'':16s} | {'t90':>5s} {'sobre%':>7s} {'err.fin':>8s} {'IAE':>7s} | {'t90':>5s} {'sobre%':>7s} {'err.fin':>8s} {'IAE':>7s} | {'t90':>5s} {'sobre%':>7s} {'err.fin':>8s} {'IAE':>7s}")
    with open(os.path.join(ROOT, "data", "steps", "metricas_degraus.csv"), "w", newline="") as f:
        w = csv.writer(f); w.writerow(["teste", "degrau", "amplitude", "t90_s", "sobressinal_pct", "erro_final_graus", "IAE_graus_s"])
        for n in nomes:
            r = todos[n][4]
            print(f"{n:16s} | " + " | ".join(f"{r[s]['t90']:5.2f} {r[s]['sobre']:7.1f} {r[s]['erro_final']:8.2f} {r[s]['iae']:7.1f}" for s, _ in STEPS))
            for s, _ in STEPS:
                w.writerow([n, s, f"{r[s]['amp']:.2f}", f"{r[s]['t90']:.2f}", f"{r[s]['sobre']:.1f}", f"{r[s]['erro_final']:.2f}", f"{r[s]['iae']:.1f}"])
    # IAE total (soma das tres janelas)
    print("\nIAE total (3 janelas de 12 s):")
    tot = {n: sum(todos[n][4][s]["iae"] for s, _ in STEPS) for n in nomes}
    for n in sorted(nomes, key=lambda x: tot[x]): print(f"  {n:16s} {tot[n]:7.1f}")
    # graficos
    fig, ax = plt.subplots(2, 1, figsize=(8.5, 5.4), sharex=True)
    for c, n in zip(cores, nomes):
        t, a, o, on, _ = todos[n]
        ax[0].plot(t - on[0], a, color=c, lw=.9, label=LABEL.get(n, n)); ax[1].plot(t - on[0], o, color=c, lw=.9)
    for e, v in zip(todos[nomes[0]][3], (10.7, -19.3, 0.7)): pass
    on = todos[nomes[0]][3]; sp = [0.7, 10.7, -19.3, 0.7]; ts = [-8, 0, on[1] - on[0], on[2] - on[0], 26]
    ax[0].step([-8, 0, on[1] - on[0], on[2] - on[0], 26], [0.7, 10.7, -19.3, 0.7, 0.7], where="post", color="k", ls=":", lw=.9, label="setpoint")
    ax[0].set_ylabel("Ângulo (°)"); ax[1].set_ylabel("Saída (%)"); ax[1].set_xlabel("Tempo desde o primeiro degrau (s)")
    ax[0].legend(fontsize=7, ncol=3); [x.grid(alpha=.3) for x in ax]
    fig.tight_layout(); fig.savefig(os.path.join(OUT, "refino_degraus.pdf")); plt.close(fig)
    # metricas por degrau
    fig, axs = plt.subplots(1, 3, figsize=(9.5, 3.2))
    x = np.arange(len(nomes)); tick = [n.split("_")[0].replace("D", "T") for n in nomes]
    for a_, key, tit in zip(axs, ("iae", "sobre", "t90"), ("IAE total (° s, 3 janelas)", "Sobressinal médio (%)", "Tempo até 90% médio (s)")):
        if key == "iae":
            vals = [tot[n] for n in nomes]
        else:
            vals = [np.nanmean([todos[n][4][s][key] for s, _ in STEPS]) for n in nomes]
        a_.bar(x, vals, color=cores); a_.set_xticks(x); a_.set_xticklabels(tick); a_.set_title(tit, fontsize=9); a_.grid(alpha=.3, axis="y")
    fig.tight_layout(); fig.savefig(os.path.join(OUT, "refino_degraus_metricas.pdf")); plt.close(fig)


if __name__ == "__main__":
    main()
