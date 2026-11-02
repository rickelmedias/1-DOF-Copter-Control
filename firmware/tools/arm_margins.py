#!/usr/bin/env python3
"""Margens de estabilidade do controle do braco (resposta em frequencia).

Malha: PID com derivada filtrada x braco. O braco e um integrador duplo (ganho
G, em graus/s2 por 1 % de PWM) com o atraso do empuxo (tau_m) e um atraso puro
(amostragem + filtro do MPU6050). Margem de fase abaixo de ~30 graus = oscila.

Uso: python3 tools/arm_margins.py
"""
import cmath, math
def loop(w, kp, ki, kd, G, tm, td=0.009, tf=0.02):
    s=1j*w
    C=kp+ki/s+kd*s/(tf*s+1) if tf>0 else kp+ki/s+kd*s
    P=G*cmath.exp(-td*s)/(s*s*(tm*s+1))
    return C*P
def margins(kp,ki,kd,G,tm,tf=0.02):
    ws=[10**(x/200.0) for x in range(-200,461)]       # 0.1 a 200 rad/s
    L=[loop(w,kp,ki,kd,G,tm,tf=tf) for w in ws]
    ph=[cmath.phase(x) for x in L]
    # desenrola a fase
    un=[ph[0]]
    for k in range(1,len(ph)):
        d=ph[k]-ph[k-1]
        while d>math.pi: d-=2*math.pi
        while d<-math.pi: d+=2*math.pi
        un.append(un[-1]+d)
    # ajusta para comecar perto de -270..-90 (integrador triplo em baixa freq.)
    while un[0]>0: un=[u-2*math.pi for u in un]
    pm=None; wc=None; gm=None
    for k in range(len(ws)-1):
        if abs(L[k])>=1>abs(L[k+1]):                  # ultimo cruzamento de ganho
            wc=ws[k]; pm=math.degrees(un[k])+180
    for k in range(len(ws)-1):
        if un[k]>-math.pi>=un[k+1] and ws[k]>(wc or 0)*0.5:
            g=-20*math.log10(abs(L[k])); gm=g if gm is None else min(gm,g)
    return wc,pm,gm
if __name__=="__main__":
    print("planta ajustada ao registro novo: G~300, atraso do empuxo ~0,058 s, +9 ms")
    for name,(kp,ki,kd,tf) in {"06/10 cedo: 1.00/0.50/0.15":(1.0,0.5,0.15,0.02),"gravado agora: 0.30/0.15/0.09":(0.30,0.15,0.09,0.02)}.items():
        for G,tm in ((300,0.058),(170,0.068)):
            wc,pm,gm=margins(kp,ki,kd,G,tm,tf=tf)
            print(f"  {name:32s} G={G} tm={tm}: cruzamento {wc:5.1f} rad/s, margem de fase {pm:6.1f} graus, margem de ganho {gm if gm is None else round(gm,1)} dB")
    best=[]
    for kp in (0.03,0.04,0.05,0.06,0.08,0.10,0.12,0.15,0.20):
        for kd in (0.015,0.02,0.025,0.03,0.035,0.04,0.05,0.06):
            for tf in (0.0,0.01,0.02):
                ki=kp*0.4
                worst_pm=1e9; worst_gm=1e9; wcs=[]
                for G in (120,200,300,380):
                    for tm in (0.045,0.058,0.075):
                        wc,pm,gm=margins(kp,ki,kd,G,tm,tf=tf)
                        if wc is None or pm is None: worst_pm=-1; break
                        worst_pm=min(worst_pm,pm); worst_gm=min(worst_gm,gm if gm is not None else 99); wcs.append(wc)
                if worst_pm>=35 and worst_gm>=6:
                    best.append((min(wcs),worst_pm,worst_gm,kp,ki,kd,tf))
    best.sort(reverse=True)
    print("\ncandidatos com margem de fase >= 35 graus e de ganho >= 6 dB em 12 plantas (G 120 a 380, atraso 0,045 a 0,075 s),\nordenados pela banda minima (maior = reage mais rapido):")
    for wc,pm,gm,kp,ki,kd,tf in best[:10]: print(f"  kp {kp:.2f} ki {ki:.3f} kd {kd:.3f} filtro D {tf:.2f}s -> banda minima {wc:4.1f} rad/s, pior margem de fase {pm:4.1f} graus, de ganho {gm:4.1f} dB")
