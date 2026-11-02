#!/usr/bin/env python3
"""Corrente e aquecimento de um motor coreless acionado por PWM.

Modelo eletrico: L di/dt = v - e - i*R. A helice mantem a rotacao constante
dentro de um periodo de PWM, entao a fcem 'e' e fixa. Para cada caso procura
o duty que da a mesma corrente media (= mesmo torque = mesmo empuxo).
"""
import math

RM = 2.0            # ohm, medido no motor (menor valor do multimetro)
TAU_E = 3.4e-6      # s, L/R de coreless de 8 mm (maxon DCX 8 M: 13,9 uH / 4,13 ohm)
I_HOVER = 0.40      # A, corrente media para pairar (HIPOTESE; medir com 2 pilhas AA)
E_HOVER = 1.70      # V, fcem ao pairar (HIPOTESE coerente com ~2,5 V medios)

def run(vm, f, mode, l_extra=0.0, r_extra=0.0, r_hs=0.28, r_ls=0.28, vdrop=0.0, vd=0.5,
        lm=None, i_target=I_HOVER, e=E_HOVER):
    lm = RM * TAU_E if lm is None else lm
    L = lm + l_extra
    T = 1.0 / f
    n = 4000
    dt = T / n

    def cycle(duty):
        i = 0.0
        res = None
        for rep in range(60):                      # ate regime permanente
            s1 = s2 = sdrive = 0.0
            ipk = -1e9
            i0 = i
            for k in range(n):
                on = (k + 0.5) / n < duty
                if on:
                    v, r = vm - vdrop, RM + r_extra + r_hs + r_ls
                elif mode == "freio":              # slow decay: motor em curto pelos transistores
                    v, r = 0.0, RM + r_extra + 2 * r_ls
                elif mode == "diodo":              # MOSFET + diodo em paralelo: sem corrente reversa
                    if i > 0:
                        v, r = -vd, RM + r_extra
                    else:
                        v, r = e, 1e9              # aberto
                else:                              # roda livre: diodos devolvem para a fonte
                    if i > 0:
                        v, r = -(vm + 2 * vd), RM + r_extra
                    else:
                        v, r = e, 1e9
                # passo exato do circuito RL
                iinf = (v - e) / r
                tau = L / r
                inew = iinf + (i - iinf) * math.exp(-dt / tau)
                if mode != "freio" and not on and i > 0 and inew < 0:
                    inew = 0.0
                im = 0.5 * (i + inew)
                s1 += im; s2 += im * im
                if on: sdrive += im
                ipk = max(ipk, inew)
                i = inew
            if abs(i - i0) < 1e-5:
                break
        return s1 / n, s2 / n, sdrive / n, ipk

    lo, hi = 0.001, 0.999
    for _ in range(40):
        mid = 0.5 * (lo + hi)
        if cycle(mid)[0] < i_target: lo = mid
        else: hi = mid
    d = 0.5 * (lo + hi)
    iav, i2, ivm, ipk = cycle(d)
    return d, iav, math.sqrt(i2), i2 * RM, ivm, ipk

P_DC = I_HOVER ** 2 * RM
print(f"Motor: R = {RM} ohm, L = {RM*TAU_E*1e6:.1f} uH (tau = {TAU_E*1e6:.1f} us). "
      f"Pairar: {I_HOVER} A medios, fcem {E_HOVER} V  ->  em corrente continua pura: {P_DC:.2f} W no motor\n")
print(f"{'caso':58s} {'duty':>5s} {'Irms':>6s} {'Ipico':>6s} {'P motor':>8s} {'x CC':>5s} {'I fonte':>8s}")
cases = [
    ("HOJE: DRV8871 12 V 100 kHz freio",                 dict(vm=12, f=100e3, mode="freio")),
    ("DRV8871 12 V 100 kHz roda livre",                  dict(vm=12, f=100e3, mode="livre")),
    ("DRV8871 12 V 20 kHz freio",                        dict(vm=12, f=20e3,  mode="freio")),
    ("DRV8871 12 V 1 kHz freio",                         dict(vm=12, f=1e3,   mode="freio")),
    ("Arduino: L298N 12 V 1 kHz roda livre (queda 2 V)", dict(vm=12, f=1e3,   mode="livre", vdrop=2.0, r_hs=0, r_ls=0, r_extra=0.6)),
    ("Arduino: L298N 6 V 1 kHz roda livre (queda 2 V)",  dict(vm=6,  f=1e3,   mode="livre", vdrop=2.0, r_hs=0, r_ls=0, r_extra=0.6)),
    ("TB6612 5 V 100 kHz freio, SEM indutor",            dict(vm=5,  f=100e3, mode="freio", r_hs=0.25, r_ls=0.25)),
    ("TB6612 5 V 20 kHz freio, SEM indutor",             dict(vm=5,  f=20e3,  mode="freio", r_hs=0.25, r_ls=0.25)),
    ("MOSFET+diodo 5 V 20 kHz, SEM indutor",             dict(vm=5,  f=20e3,  mode="diodo", r_hs=0.02, r_ls=0.02)),
    ("DRV8871 12 V 100 kHz freio + indutor 100 uH",      dict(vm=12, f=100e3, mode="freio", l_extra=100e-6, r_extra=0.15)),
    ("DRV8871 12 V 100 kHz freio + indutor 220 uH",      dict(vm=12, f=100e3, mode="freio", l_extra=220e-6, r_extra=0.25)),
    ("DRV8871 12 V 100 kHz freio + indutor 47 uH",       dict(vm=12, f=100e3, mode="freio", l_extra=47e-6,  r_extra=0.10)),
    ("TB6612 5 V 100 kHz freio + indutor 100 uH",        dict(vm=5,  f=100e3, mode="freio", r_hs=0.25, r_ls=0.25, l_extra=100e-6, r_extra=0.15)),
    ("TB6612 5 V 20 kHz freio + indutor 100 uH",         dict(vm=5,  f=20e3,  mode="freio", r_hs=0.25, r_ls=0.25, l_extra=100e-6, r_extra=0.15)),
]
for name, kw in cases:
    d, iav, irms, pm, ivm, ipk = run(**kw)
    print(f"{name:58s} {d*100:4.0f}% {irms:6.2f} {ipk:6.2f} {pm:7.2f}W {pm/P_DC:5.1f} {ivm:7.3f}A")

print("\nSensibilidade do caso de HOJE a indutancia do motor (nao medida):")
for lm in (3.5e-6, 7e-6, 15e-6, 30e-6):
    d, iav, irms, pm, ivm, ipk = run(vm=12, f=100e3, mode="freio", lm=lm)
    d2, _, irms2, pm2, ivm2, ipk2 = run(vm=12, f=100e3, mode="freio", lm=lm, l_extra=100e-6, r_extra=0.15)
    print(f"  L = {lm*1e6:4.1f} uH: P motor = {pm:5.2f} W, pico {ipk:4.2f} A, fonte {ivm:5.3f} A   |  com indutor 100 uH: {pm2:4.2f} W, pico {ipk2:4.2f} A")

print("\nPartida (motor parado, fcem = 0) no limite de 32 % de duty, pico de corrente (DRV8871 desarma em ~3,7 A):")
for name, kw in (("12 V 100 kHz freio", dict(vm=12, f=100e3)), ("12 V 20 kHz freio", dict(vm=12, f=20e3)),
                 ("12 V 1 kHz freio", dict(vm=12, f=1e3)), ("12 V 100 kHz freio + 100 uH", dict(vm=12, f=100e3, l_extra=100e-6, r_extra=0.15))):
    # corrente de pico com duty fixo de 32 % e e = 0
    L = RM*TAU_E + kw.get("l_extra", 0.0); r_on = RM + kw.get("r_extra",0)+0.565; r_off = RM + kw.get("r_extra",0)+0.56
    T = 1/kw["f"]; ton = 0.32*T; toff = T-ton
    a = math.exp(-ton*r_on/L); b = math.exp(-toff*r_off/L); iinf = kw["vm"]/r_on
    ipk = iinf*(1-a)/(1-a*b)
    print(f"  {name:32s} pico = {ipk:4.2f} A")
