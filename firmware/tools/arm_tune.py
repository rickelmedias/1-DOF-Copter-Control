#!/usr/bin/env python3
"""Simula o controle do firmware em cima do braco medido e procura ganhos.

O braco foi identificado pelo registro de 06/10/2026 (tools/dbg_read.py --log):
com Kp 1,0 / Ki 0,5 / Kd 0,15 ele oscilou de -10,8 a +5,4 graus com periodo de
0,45 s e a saida batendo em 10 % e 32 %. O modelo abaixo reproduz isso com
~140 graus/s2 por 1 % de PWM e ~0,07 s de atraso do empuxo.

Uso, a partir da pasta do projeto:
    python3 tools/arm_tune.py            compara os ganhos de hoje com os melhores
    python3 tools/arm_tune.py calibra    mostra a calibracao do modelo
"""

import math, random, sys

def load(path):
    rows=[]
    for l in open(path):
        f=l.split()
        if len(f)==5:
            try: rows.append([float(x) for x in f])
            except: pass
    return rows

def simulate(kp, ki, kd, G=115.0, tau_m=0.07, hover=21.0, tau_f=0.02, slew=300.0, idle=10.0, lim=32.0,
             preload=0.0, delay_steps=2, t_end=20.0, noise=0.0, quad=True, th0=-38.0, sp=0.7, seed=1, damp=0.3,
             kick=None):
    """Mesma logica de Control_Step/Pid_Step. Planta: empuxo com atraso de 1a ordem.
    quad=True: empuxo proporcional ao quadrado da saida (helice), com ganho local G em 'hover'."""
    random.seed(seed)
    dt=0.005; n=int(t_end/dt)
    th=th0; w=0.0; thrust_u=0.0          # thrust_u: saida "vista" pela helice (atrasada)
    integ=preload; out=0.0; rate_f=0.0
    hist=[(th,w)]*(delay_steps+1)
    log=[]
    for k in range(n):
        th_m,w_m=hist[0]
        w_meas=w_m+(random.uniform(-noise,noise) if noise else 0.0)
        rate_f+=dt/(tau_f+dt)*(w_meas-rate_f)
        err=sp-th_m
        u=kp*err+integ-kd*rate_f
        if not((u>lim and err>0) or (u<idle and err<0)):
            integ=min(max(integ+ki*err*dt,0.0),lim)
        req=min(max(u,idle),lim)
        step=slew*dt
        out+=min(max(req-out,-step),step)
        # planta
        thrust_u+=(out-thrust_u)*dt/tau_m
        if quad:
            acc=G*hover/2.0*((thrust_u/hover)**2-1.0)      # dAcc/du = G em u = hover
        else:
            acc=G*(thrust_u-hover)
        acc-=damp*w
        if kick and kick[0]<=k*dt<kick[0]+kick[1]: acc+=kick[2]
        w+=acc*dt; th+=w*dt
        if th<-38.0: th=-38.0; w=max(w,0.0)
        hist=hist[1:]+[(th,w)]
        log.append((k*dt,th,w,out))
    return log

def describe(log,t0):
    seg=[r for r in log if r[0]>=t0]
    a=[r[1] for r in seg]; o=[r[3] for r in seg]
    m=sum(a)/len(a)
    z=[seg[k][0] for k in range(len(seg)-1) if (a[k]-m)<0<=(a[k+1]-m)]
    per=(z[-1]-z[0])/(len(z)-1) if len(z)>2 else float('nan')
    return min(a),max(a),per,min(o),max(o),sum(abs(o[k+1]-o[k]) for k in range(len(o)-1))/(len(o)-1)


def calibra():
    print("Ganhos de 06/10 (kp 1.0, ki 0.5, kd 0.15). Medido: -10.8 a +5.4 graus, periodo 0,45 s, saida 10 a 32")
    for G in (90,115,140):
        for tm in (0.05,0.07,0.09):
            lo,hi,per,omin,omax,_=describe(simulate(1.0,0.5,0.15,G=G,tau_m=tm),12.0)
            print(f"  G={G:3d} tau_m={tm:.2f}: angulo {lo:6.1f} a {hi:5.1f}  periodo {per:.2f} s  saida {omin:.0f} a {omax:.0f}")

PLANTS=[(G,tm) for G in (100,140,190) for tm in (0.05,0.07,0.10)]
def score(kp,ki,kd,tau_f=0.02,preload=18.0,verbose=False):
    worst=0.0; rows=[]
    for G,tm in PLANTS:
        log=simulate(kp,ki,kd,G=G,tau_m=tm,tau_f=tau_f,preload=preload,noise=30.0,t_end=14.0,kick=(8.0,0.15,400.0))
        th=[r[1] for r in log]; out=[r[3] for r in log]; t=[r[0] for r in log]
        over=max(th)-0.7                                   # passou do alvo (decolagem + empurrao)
        # erro medio absoluto de 4 a 8 s (assentado) e de 9.5 a 14 s (depois do empurrao)
        e1=sum(abs(x-0.7) for x,tt in zip(th,t) if 4<=tt<8)/800.0
        e2=sum(abs(x-0.7) for x,tt in zip(th,t) if 9.5<=tt<14)/900.0
        chat=sum(abs(out[k+1]-out[k]) for k in range(1600,2799))/1199.0   # % por ciclo, de 8 a 14 s
        rise=next((tt for x,tt in zip(th,t) if x>-3.0),99.0)
        s=e1*2+e2*2+max(over-6.0,0)*0.5+chat*4+max(rise-2.5,0)*1.0
        worst=max(worst,s); rows.append((G,tm,over,e1,e2,chat,rise))
    if verbose:
        for r in rows: print("   G=%3d tau_m=%.2f: passa do alvo %5.1f graus | erro medio %.2f / %.2f graus | saida varia %.2f %%/ciclo | sobe em %.1f s"%r)
    return worst
if __name__=="__main__":
    if len(sys.argv)>1 and sys.argv[1]=="calibra":
        calibra(); sys.exit(0)
    print("hoje  kp 1.00 ki 0.50 kd 0.15 (sem pre-carga):"); print("  nota %.2f"%score(1.0,0.5,0.15,preload=0.0,verbose=True))
    best=[]
    for kp in (0.10,0.15,0.20,0.25,0.30,0.40,0.50,0.65):
        for kd in (0.03,0.04,0.05,0.06,0.075,0.09,0.11):
            for kir in (0.25,0.5,1.0):
                for tf in (0.01,0.02):
                    ki=kp*kir
                    best.append((score(kp,ki,kd,tau_f=tf),kp,ki,kd,tf))
    best.sort()
    print("\nmelhores (nota = pior caso entre 9 bracos; menor e melhor):")
    for s,kp,ki,kd,tf in best[:8]: print("  nota %.2f  kp %.2f ki %.3f kd %.3f  filtro D %.2f s"%(s,kp,ki,kd,tf))
    s,kp,ki,kd,tf=best[0]
    print("\ndetalhe do melhor:"); score(kp,ki,kd,tau_f=tf,verbose=True)
