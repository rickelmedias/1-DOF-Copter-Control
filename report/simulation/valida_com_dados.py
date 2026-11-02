#!/usr/bin/env python3
"""Confronta a simulacao com a partida real e refaz o PID sobre os dados medidos.

Entrada: data/raw_readings/partida_apos_reset.txt e motor_ligado_pairando.txt (saida de
firmware/tools/dbg_read.py --log, 20 Hz). Gera images/results/partida_real_vs_sim.pdf,
filtro_partida.pdf e filtro_pairando.pdf e imprime as metricas usadas no texto.
"""
import os, sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
sys.path.insert(0, os.path.dirname(__file__))
import sim_1dof as S

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")   # raiz do repositorio
OUTDIR = S.OUT_DIR


def load(path):
    rows = []
    for line in open(path, encoding="utf-8", errors="ignore"):
        f = line.split()
        if len(f) == 6:
            try:
                rows.append([float(x) for x in f])
            except ValueError:
                pass
    return np.array(rows).T          # t, angulo, veloc, ang_acel, acel_g, saida


def entra_faixa(t, ang, band=2.0):
    ok = np.abs(ang - S.SP) <= band
    for i in range(len(t)):
        if ok[i:].all():
            return t[i] - t[0]
    return float("nan")


def filtro(path, nome, titulo):
    t, ang, rate, acc, g, out = load(path)
    fig, ax = plt.subplots(2, 1, figsize=(7.5, 5), sharex=True)
    ax[0].plot(t, acc, color="tab:red", lw=.8, label="sem filtro (só acelerômetro)")
    ax[0].plot(t, ang, color="tab:blue", lw=1.4, label="firmware (giroscópio integrado em voo)")
    ax[0].set_ylabel("Ângulo (°)"); ax[0].legend(fontsize=8); ax[0].grid(alpha=.3); ax[0].set_title(titulo, fontsize=10)
    ax[1].plot(t, out, "k"); ax[1].set_ylabel("Saída (%)"); ax[1].set_xlabel("Tempo (s)"); ax[1].grid(alpha=.3)
    fig.tight_layout(); fig.savefig(os.path.join(OUTDIR, nome)); plt.close(fig)
    return t, ang, rate, acc, out


def main():
    os.makedirs(OUTDIR, exist_ok=True)
    # ---- pairando
    t, ang, rate, acc, out = filtro(os.path.join(ROOT, "data", "raw_readings", "motor_ligado_pairando.txt"),
                                    "filtro_pairando.pdf", "Haste pairando (motor ligado): acelerômetro contra ângulo do firmware")
    print(f"PAIRANDO N={len(t)} {t[-1]-t[0]:.1f} s | firmware: media {ang.mean():.2f} desvio {ang.std():.2f} min {ang.min():.2f} max {ang.max():.2f}"
          f" | acelerometro: media {acc.mean():.2f} desvio {acc.std():.2f} min {acc.min():.1f} max {acc.max():.1f}"
          f" | RMS(acel-firmware) {np.sqrt(np.mean((acc-ang)**2)):.2f} | saida media {out.mean():.2f} (min {out.min():.1f} max {out.max():.1f})")
    # ---- partida
    t, ang, rate, acc, out = filtro(os.path.join(ROOT, "data", "raw_readings", "partida_apos_reset.txt"),
                                    "filtro_partida.pdf", "Partida após o reset: acelerômetro contra ângulo do firmware")
    rep = out < 5.0
    print(f"PARTIDA N={len(t)} {t[-1]-t[0]:.1f} s | saida max {out.max():.1f} % | angulo inicial {ang[1]:.1f}, final {ang[-1]:.1f}"
          f" | entra e fica a +-2 graus do alvo em {entra_faixa(t, ang):.2f} s apos o inicio do registro")
    voo = t > t[0] + 1.0
    print(f"  em voo: RMS(acel-firmware) {np.sqrt(np.mean((acc[voo]-ang[voo])**2)):.1f} graus; desvio do acelerometro em torno do firmware {np.std(acc[voo]-ang[voo]):.1f}")
    print(f"  em repouso (primeiras amostras, saida<5%): {rep.sum()} amostra(s)")
    # ---- simulacao contra o real (hover medido 20,2 %)
    for hover in (0.183, 0.202):
        G = 300.0 * 100.0 * hover
        T, TH, OUT, _ = S.run("cond", t_end=12.0, G=G, hover=hover, rest=ang[0])
        print(f"SIM hover {hover*100:.1f}%: a +-2 graus em {S.metrics(T, TH, 0.0)[1]:.2f} s, saida final {OUT[-1]:.1f}")
        if hover == 0.202:
            Ts, THs, OUTs = T, TH, OUT
    fig, ax = plt.subplots(2, 1, figsize=(7.5, 5), sharex=True)
    ax[0].plot(t - t[0], ang, "tab:blue", label="bancada"); ax[0].plot(Ts, THs, "tab:orange", ls="--", label="simulação (pairar em 20,2%)")
    ax[0].axhline(S.SP, color="k", ls=":", lw=.8); ax[0].set_ylabel("Ângulo (°)"); ax[0].legend(fontsize=8); ax[0].grid(alpha=.3)
    ax[1].plot(t - t[0], out, "tab:blue"); ax[1].plot(Ts, OUTs, "tab:orange", ls="--"); ax[1].set_ylabel("Saída (%)"); ax[1].set_xlabel("Tempo (s)"); ax[1].grid(alpha=.3)
    fig.tight_layout(); fig.savefig(os.path.join(OUTDIR, "partida_real_vs_sim.pdf")); plt.close(fig)
    # ---- PID refeito sobre os dados medidos, com e sem integracao condicional
    dt = S.DT
    tt = np.arange(t[0], t[-1], dt)
    A = np.interp(tt, t, ang); R = np.interp(tt, t, rate)
    res = {}
    for mode in ("cond", "none"):
        c = S.Controller(mode); o = []; ig = []
        for a_, r_ in zip(A, R):
            o.append(c.step(a_, r_)); ig.append(c.integ)
        res[mode] = (np.array(o), np.array(ig))
    d = np.abs(res["cond"][0] - res["none"][0]).max()
    print(f"REPLAY: saida maxima do PID refeito {res['cond'][0].max():.1f} %, limite {S.OUT_LIMIT} %; diferenca maxima entre com e sem anti-windup {d:.4f} pontos")
    print(f"        integrador final {res['cond'][1][-1]:.2f} %  (saida real final {out[-1]:.1f} %)")


if __name__ == "__main__":
    main()
