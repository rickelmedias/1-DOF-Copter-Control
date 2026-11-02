#!/usr/bin/env python3
"""Le o diagnostico do firmware (g_debug, debug.h) pelo ST-Link, sem serial.

Uso, a partir da pasta do projeto:
    python3 tools/dbg_read.py            contadores e estado atual
    python3 tools/dbg_read.py --log      tambem o registro (angulo, velocidade, acelerometro, saida)
    python3 tools/dbg_read.py --reset    reinicia a placa (zera o registro) e sai
    python3 tools/dbg_read.py --set kp=1.2 kd=0.1     muda valores com o firmware rodando

Com --set (sem serial, sem regravar e sem reiniciar o motor):
    kp= ki= kd=     ganhos do PID
    sp=             setpoint, em graus
    motor=0|1       desliga / liga o motor
    aw=0|1          anti-windup: 1 = integracao condicional (normal), 0 = so limita o integrador (ensaio)
    logdiv=N        1 amostra do registro a cada N ciclos de 5 ms e recomeca o
                    registro (1 = 200 Hz, guarda 1,6 s; 10 = 20 Hz, guarda 16 s)
Os valores voltam aos de app_config.h no proximo reset.

Conecta em hot plug: nao para nem reinicia o firmware (a nao ser com --reset).
"""
import glob
import math
import re
import struct
import subprocess
import sys

ELF = "Debug/topicos2.elf"
IDE = "/opt/st/stm32cubeide_*/plugins/"
CONNECT = ["-c", "port=SWD", "freq=100", "mode=HOTPLUG"]


def tool(pattern):
    found = sorted(glob.glob(IDE + pattern))
    if not found:
        sys.exit("nao achei " + pattern)
    return found[-1]


CLI = tool("*cubeprogrammer.linux64*/tools/bin/STM32_Programmer_CLI")
NM = tool("*gnu-tools-for-stm32*/tools/bin/arm-none-eabi-nm")


def symbols():
    out = subprocess.run([NM, "-S", ELF], capture_output=True, text=True, check=True).stdout
    syms = {}
    for line in out.splitlines():
        f = line.split()
        if len(f) == 4:
            syms[f[3]] = (int(f[0], 16), int(f[1], 16))
    return syms


def cli(args):
    out = subprocess.run([CLI] + CONNECT + args, capture_output=True, text=True, timeout=90).stdout
    return re.sub(r"\x1b\[[0-9;]*m", "", out)


def read(regions):
    """regions: lista de (endereco, tamanho). Devolve um bytes por regiao."""
    args = []
    for addr, size in regions:
        args += ["-r8", hex(addr), str(size)]
    out = cli(args)
    mem = {}
    for line in out.splitlines():
        m = re.match(r"^0x([0-9A-Fa-f]{8})\s*:\s*(.*)$", line)
        if m:
            base = int(m.group(1), 16)
            for i, b in enumerate(x for x in m.group(2).split() if len(x) == 2):
                mem[base + i] = int(b, 16)
    result = []
    for addr, size in regions:
        if any(a not in mem for a in range(addr, addr + size)):
            sys.exit("leitura falhou:\n" + out[-600:])
        result.append(bytes(mem[a] for a in range(addr, addr + size)))
    return result


PARAM_OFFSET = {"kp": 0, "ki": 4, "kd": 8, "sp": 12}    # campos float de CtrlParams_t
ARMED_OFFSET = 16                                        # CtrlParams_t.armed (bool)
AW_OFFSET = 17                                           # CtrlParams_t.anti_windup (bool)
TIMING_BYTES = 48 + 16 + 2 * 32 * 4                       # fim de DebugData_t: tempos, heap, calibracao, corte e dois histogramas
LOG_DIV_OFFSET = 40                                      # DebugData_t.log_div (uint16)
LOG_N_OFFSET = 44                                        # DebugData_t.log_n (uint32)


def write_values(syms, pairs):
    """Escreve na RAM do firmware os valores pedidos em --set."""
    params_addr = syms["s_params"][0]
    args = []
    for pair in pairs:
        name, sep, text = pair.partition("=")
        if not sep:
            sys.exit("use nome=valor, por exemplo kp=1.2")
        if name in PARAM_OFFSET:
            value = float(text)
            if name != "sp" and value < 0.0:
                sys.exit("ganho negativo nao pode")
            if name == "sp" and abs(value) > 60.0:
                sys.exit("setpoint fora de +-60 graus")
            word = struct.unpack("<I", struct.pack("<f", value))[0]
            args += ["-w32", hex(params_addr + PARAM_OFFSET[name]), "0x%08X" % word]
        elif name == "motor":
            args += ["-w8", hex(params_addr + ARMED_OFFSET), "0x01" if int(text) != 0 else "0x00"]
        elif name == "aw":
            args += ["-w8", hex(params_addr + AW_OFFSET), "0x01" if int(text) != 0 else "0x00"]
        elif name == "logdiv":
            div = int(text)
            if not 1 <= div <= 1000:
                sys.exit("logdiv de 1 a 1000")
            if "g_debug" not in syms:
                sys.exit("o firmware foi compilado com DEBUG_ENABLE 0")
            dbg_addr = syms["g_debug"][0]
            args += ["-w16", hex(dbg_addr + LOG_DIV_OFFSET), "0x%04X" % div,
                     "-w32", hex(dbg_addr + LOG_N_OFFSET), "0x00000000"]
        else:
            sys.exit("nao conheco '%s' (use kp, ki, kd, sp, motor, aw ou logdiv)" % name)
    out = cli(args)
    if "rror" in out:
        sys.exit("escrita falhou:\n" + out[-600:])


def main():
    if "--reset" in sys.argv:
        print("\n".join(l for l in cli(["-rst"]).splitlines() if "eset" in l or "rror" in l))
        return

    syms = symbols()
    if "--set" in sys.argv:
        pairs = [a for a in sys.argv[sys.argv.index("--set") + 1:] if not a.startswith("--")]
        if not pairs:
            sys.exit("faltou o que mudar, por exemplo: --set kp=1.2")
        write_values(syms, pairs)
    want_log = "--log" in sys.argv
    head = 48                                  # bytes de g_debug antes do registro
    dbg_addr, dbg_size = syms["g_debug"]
    regions = [syms["s_params"], syms["s_telem"], syms["s_state"], syms["uwTick"],
               (dbg_addr, dbg_size if want_log else head),
               (dbg_addr + dbg_size - TIMING_BYTES, TIMING_BYTES)]
    params, telem, state, tick, dbg, timing = read(regions)

    kp, ki, kd, sp, armed, aw = struct.unpack_from("<ffffBB", params)
    ang, spc, out = struct.unpack_from("<fff", telem)
    (read_fail, recover, uart_err, samples, amax, amin, rmax,
     s0, s1, s2, log_div, log_size, log_n) = struct.unpack_from("<4I3f3I2HI", dbg)

    print("ligado ha %.1f s   estado=%s" % (
        struct.unpack("<I", tick)[0] / 1000.0,
        {0: "sem sensor (motor solto)", 1: "calibrando", 2: "rodando"}.get(state[0], state[0])))
    print("ganhos: kp=%.2f ki=%.2f kd=%.2f   alvo=%.1f   armado=%d   anti-windup=%d" % (kp, ki, kd, sp, armed, aw))
    print("agora:  angulo=%.1f   setpoint=%.1f   saida=%.1f%%" % (ang, spc, out))
    print("falhas de leitura=%d   reinicios do I2C=%d   erros da UART=%d"
          % (read_fail, recover, uart_err))
    print("amostras vistas: %d" % samples)
    print("angulo min=%.1f max=%.1f   velocidade max=%.0f graus/s" % (amin, amax, rmax))
    print("folga de pilha (palavras): controle=%d comandos=%d telemetria=%d" % (s0, s1, s2))

    (ticks, takes, pmin, pmax, wake, execm, lat, lat_sum, lat_n, miss, heap, heap_min,
     bias, start_ang, cut_ang, cut_n) = struct.unpack_from("<12I3fI", timing)
    hist_lat = struct.unpack_from("<32I", timing, 64)
    hist_per = struct.unpack_from("<32I", timing, 64 + 128)
    us = 72.0
    print("tempos (DWT, 72 MHz): ticks TIM2=%d  acordadas=%d  perdidos=%d  atrasos de prazo=%d" % (ticks, takes, ticks - takes, miss))
    print("  intervalo entre ticks: min %.1f us  max %.1f us" % (pmin / us, pmax / us))
    print("  interrupcao -> tarefa acordada: max %.1f us   tarefa -> PWM escrito: max %.1f us" % (wake / us, execm / us))
    print("  interrupcao -> PWM escrito: max %.1f us   media %.1f us (%d ciclos)" % (lat / us, lat_sum / max(lat_n, 1), lat_n))
    print("heap: livre %d bytes   menor folga %d bytes" % (heap, heap_min))
    print("calibracao: bias do giroscopio %.4f graus/s   angulo inicial %.2f   cortes por angulo: %d (ultimo em %.1f graus)"
          % (bias, start_ang, cut_n, cut_ang))
    print("hist_latencia_50us:", " ".join(str(x) for x in hist_lat))
    print("hist_intervalo_1us_desde_4990:", " ".join(str(x) for x in hist_per))

    if want_log:
        dt = log_div * 0.005
        count = min(log_n, log_size)
        first = log_n - count                  # indice da amostra mais antiga
        print("registro: ultimas %d amostras, uma a cada %.0f ms" % (count, dt * 1000))
        print("log_n=%d primeiro_indice=%d" % (log_n, first))
        # ang.acel: angulo so do acelerometro, com o mesmo +180 do firmware (ANGLE_OFFSET_DEG)
        print("  tempo  angulo  veloc.  ang.acel  acel(g)  saida   (tempo 0 = ultima amostra)")
        for k in range(count):
            a, r, x, z, o = struct.unpack_from("<5h", dbg, head + 10 * ((first + k) % log_size))
            acc = (math.degrees(math.atan2(x, z)) + 180.0 + 180.0) % 360.0 - 180.0
            print("%6.2f  %6.1f  %6.1f  %8.1f  %6.2f  %5.1f"
                  % ((k - count + 1) * dt, a / 10, r / 10, acc, math.hypot(x, z) / 1000, o / 10))

if __name__ == "__main__":
    main()
