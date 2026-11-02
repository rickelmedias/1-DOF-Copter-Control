#!/usr/bin/env python3
"""Captura ~1 minuto de registro (20 Hz) com um conjunto de ganhos, juntando varias leituras.

O registro da RAM guarda so 16 s; este script le a placa repetidamente (tools/dbg_read.py),
costura as leituras pelo indice absoluto da amostra (log_n) e grava um CSV.

Uso, a partir da pasta firmware/:
    python3 tools/captura_longa.py NOME kp ki kd aw [settle_s] [record_s]
Exemplo:
    python3 tools/captura_longa.py A_atual 0.06 0.05 0.04 1 25 60
Grava data/tests/NOME.csv com: indice, t_s, angulo, velocidade, ang_acel, acel_g, saida.
Aborta e desliga o motor se o angulo sair de +-35 graus depois da estabilizacao.
"""
import csv, os, re, subprocess, sys, time

AQUI = os.path.dirname(os.path.abspath(__file__))
DADOS = os.path.normpath(os.path.join(AQUI, "..", "..", "data"))   # raiz do repositorio / dados

HZ = 20.0   # DEBUG_LOG_DIV = 10 -> 20 Hz
DESCARTE = 60


def dbg(*args, timeout=100):
    out = subprocess.run([sys.executable, "tools/dbg_read.py", *args],
                         capture_output=True, text=True, timeout=timeout).stdout
    return out


def parse(out):
    m = re.search(r"log_n=(\d+) primeiro_indice=(\d+)", out)
    if not m:
        return None
    first = int(m.group(2))
    rows = []
    for line in out.splitlines():
        f = line.split()
        if len(f) == 6:
            try:
                rows.append([float(x) for x in f])
            except ValueError:
                pass
    return first, rows


def main():
    name, kp, ki, kd, aw = sys.argv[1:6]
    settle = float(sys.argv[6]) if len(sys.argv) > 6 else 25.0
    record = float(sys.argv[7]) if len(sys.argv) > 7 else 60.0
    out = dbg("--set", f"kp={kp}", f"ki={ki}", f"kd={kd}", f"aw={aw}", "logdiv=10")
    t0 = time.time()
    print(f"[{name}] ganhos kp={kp} ki={ki} kd={kd} aw={aw} aplicados; estabilizando {settle:.0f} s", flush=True)
    i0 = int(settle * HZ)
    n = int(record * HZ)
    got = {}
    while True:
        time.sleep(max(0.0, 8.0 - 0))
        res = parse(dbg("--log"))
        if res is None:
            continue
        first, rows = res
        for k, r in enumerate(rows):
            if k >= DESCARTE:   # as mais antigas podem ter sido sobrescritas durante a leitura (nao atomica)
                got.setdefault(first + k, r)
        if got:
            last = got[max(got)]
            if time.time() - t0 > settle and abs(last[1]) > 35.0:
                dbg("--set", "motor=0")
                print(f"[{name}] ABORTADO: angulo {last[1]:.1f}; motor desligado", flush=True)
                sys.exit(1)
        if max(got, default=0) >= i0 + n and all(i in got for i in range(i0, i0 + n)):
            break
        if time.time() - t0 > settle + record + 120:
            print(f"[{name}] ABORTADO: nao consegui amostras suficientes (faltam indices)", flush=True)
            sys.exit(1)
    os.makedirs(os.path.join(DADOS, "tests"), exist_ok=True)
    path = os.path.join(DADOS, "tests", f"{name}.csv")
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["indice", "t_s", "angulo_graus", "velocidade_dps", "ang_acel_graus", "acel_g", "saida_pct", "kp", "ki", "kd", "aw"])
        for idx in range(i0, i0 + n):
            r = got[idx]
            w.writerow([idx, (idx - i0) / HZ, r[1], r[2], r[3], r[4], r[5], kp, ki, kd, aw])
    print(f"[{name}] gravado {path} ({n} amostras, {record:.0f} s)", flush=True)


if __name__ == "__main__":
    main()
