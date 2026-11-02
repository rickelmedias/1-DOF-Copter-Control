#!/usr/bin/env python3
"""Resposta a degraus de setpoint com um conjunto de ganhos (para o refino de PID e anti-windup).

Sequencia (setpoint, em graus): 0,7 -> 10,7 (+10) -> -19,3 (-20) -> 0,7. O registro (20 Hz) e lido
repetidamente e costurado pelo indice da amostra. Os degraus sao dados com `dbg_read.py --set sp=...`.
Uso (a partir de firmware/): python3 tools/captura_degraus.py NOME kp ki kd aw
Grava data/steps/NOME.csv e data/steps/NOME_eventos.txt (indice aproximado de cada degrau).
Aborta e desliga o motor se o angulo sair de -60..+40 graus.
"""
import csv, os, re, subprocess, sys, time

AQUI = os.path.dirname(os.path.abspath(__file__))
DADOS = os.path.normpath(os.path.join(AQUI, "..", "..", "data"))   # raiz do repositorio / dados

HZ = 20.0
DESCARTE = 60   # amostras mais antigas de cada leitura descartadas (leitura nao e atomica)


def dbg(*args):
    return subprocess.run([sys.executable, "tools/dbg_read.py", *args], capture_output=True, text=True, timeout=100).stdout


def read_log(got):
    out = dbg("--log")
    m = re.search(r"log_n=(\d+) primeiro_indice=(\d+)", out)
    if not m:
        return None
    first, k = int(m.group(2)), 0
    last = None
    for line in out.splitlines():
        f = line.split()
        if len(f) == 6:
            try:
                r = [float(x) for x in f]
            except ValueError:
                continue
            if k >= DESCARTE:      # as mais antigas podem ja ter sido sobrescritas durante a leitura
                got.setdefault(first + k, r)
            last = r
            k += 1
    return int(m.group(1)), last


def main():
    name, kp, ki, kd, aw = sys.argv[1:6]
    got, events = {}, []
    dbg("--set", f"kp={kp}", f"ki={ki}", f"kd={kd}", f"aw={aw}", "sp=0.7", "logdiv=10")
    t0 = time.time()
    print(f"[{name}] kp={kp} ki={ki} kd={kd} aw={aw}", flush=True)
    # (comando, nome, segundos de espera depois do comando); a leitura do registro continua durante a espera
    plano = [(None, "base", 8.0), ("sp=10.7", "+10", 12.0), ("sp=-19.3", "-20", 12.0), ("sp=0.7", "volta", 14.0)]
    time.sleep(10)
    for cmd, nome, espera in plano:
        if cmd is not None:
            dbg("--set", cmd)
            events.append((nome, cmd, time.time() - t0))
        fim = time.time() + espera
        while time.time() < fim:
            res = read_log(got)
            if res and res[1] and not (-60.0 < res[1][1] < 40.0):
                dbg("--set", "motor=0", "sp=0.7")
                print(f"[{name}] ABORTADO: angulo {res[1][1]:.1f}", flush=True)
                sys.exit(1)
    read_log(got)
    dbg("--set", "sp=0.7")
    os.makedirs(os.path.join(DADOS, "steps"), exist_ok=True)
    with open(os.path.join(DADOS, "steps", f"{name}.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["indice", "t_s", "angulo_graus", "velocidade_dps", "ang_acel_graus", "acel_g", "saida_pct", "kp", "ki", "kd", "aw"])
        for idx in sorted(got):
            r = got[idx]
            w.writerow([idx, idx / HZ, r[1], r[2], r[3], r[4], r[5], kp, ki, kd, aw])
    with open(os.path.join(DADOS, "steps", f"{name}_eventos.txt"), "w") as f:
        for ev in events:
            f.write(f"{ev[0]} {ev[1]} instante_relogio_s={ev[2]:.1f}\n")
    print(f"[{name}] gravado ({len(got)} amostras, {len(got)/HZ:.0f} s)", flush=True)


if __name__ == "__main__":
    main()
