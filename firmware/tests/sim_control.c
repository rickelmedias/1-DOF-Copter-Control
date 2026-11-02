/*
 * sim_control.c - Teste no PC da lei de controle: usa control.c, pid.c e
 *                 angle_filter.c de verdade, com um braco simulado.
 *
 * O braco simulado usa os numeros medidos na bancada em 06/10/2026 (ver
 * app_config.h): ~300 graus/s2 por 1 % de PWM, ~0,06 s de atraso do empuxo e
 * paira com ~18 %. Os outros casos variam esses numeros em volta do medido.
 * O teste confere a logica (rampa da saida, limites, sinais) e se os ganhos
 * assentam nesses bracos; a conta fina dos ganhos esta em tools/arm_margins.py.
 *
 * Rodar, a partir da pasta do projeto:
 *   gcc -Wall -Wextra -IModules/app -IModules/control -IModules/sensor \
 *       tests/sim_control.c Modules/control/control.c Modules/control/pid.c \
 *       Modules/sensor/angle_filter.c -lm -o /tmp/sim_control && /tmp/sim_control
 *
 * Esta pasta nao entra no build do firmware.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "app_config.h"
#include "control.h"
#include "angle_filter.h"

#define DT CTRL_PERIOD_S
/* Fracao do PWM em que o braco simulado paira, relativa ao limite de saida */
#define HOVER(frac_do_limite) ((frac_do_limite) * OUT_LIMIT_PCT / 100.0)
static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { printf("  FALHOU: %s\n", msg); fails++; } } while (0)

typedef struct { double th, w, thrust; double G, hover, tau, B, rest; } Plant;

static void plant_step(Plant *pl, double out_pct) {
    double u = out_pct / 100.0; if (u < 0) u = 0;          /* helice: so empurra */
    pl->thrust += (u - pl->thrust) * DT / pl->tau;
    double A = pl->G / pl->hover;                           /* empuxo que equilibra em 0 graus */
    double acc = A * pl->thrust - pl->G * cos(pl->th * M_PI / 180.0) - pl->B * pl->w;
    pl->w += acc * DT; pl->th += pl->w * DT;
    if (pl->th < pl->rest) { pl->th = pl->rest; if (pl->w < 0) pl->w = 0; }   /* batente do repouso */
}

static void run(const char *name, double G, double hover, double tau, int verbose, int check_settle) {
    Plant pl = { -38, 0, 0, G, hover, tau, 1.0, -38 };
    Control_t c; CtrlParams_t p = { KP_DEFAULT, KI_DEFAULT, KD_DEFAULT, SETPOINT_DEFAULT_DEG, 1, 1 };
    double prev_out = 0, max_step = 0, min_out_active = 1e9, peak = -1e9;
    int n = (int)(20.0 / DT), i;
    Control_Init(&c);
    printf("== %s (G=%.0f graus/s2, paira com %.0f%%, atraso do motor %.0f ms)\n", name, G, hover*100, tau*1000);
    for (i = 0; i < n; i++) {
        double t = i * DT;
        bool lost = (t >= 12.0 && t < 12.3);                       /* sensor some por 0,3 s */
        float out = 0.0f;
        if (lost) { Control_Stop(&c); } else { out = Control_Step(&c, &p, (float)pl.th, (float)pl.w, DT); }
        CHECK(out >= 0.0f, "saida negativa (helice nao pode inverter)");
        CHECK(out <= OUT_LIMIT_PCT + 1e-3, "saida acima do limite");
        if (out > 0.0f && fabs(out - prev_out) > max_step) max_step = fabs(out - prev_out);
        if (lost) CHECK(!c.active, "sem sensor o controle tem que parar");
        if (c.active && t > 1.0 && (t < 12.0 || t > 13.0) && out < min_out_active) min_out_active = out;
        if (pl.th > peak) peak = pl.th;
        prev_out = out;
        plant_step(&pl, out);
        if (verbose && (i % 100) == 0) printf("  t=%5.1f ang=%7.2f out=%5.1f I=%5.1f\n", t, pl.th, out, c.pid.integ);
    }
    printf("  final: ang=%.2f  pico=%.2f  maior variacao por ciclo=%.3f%% (limite %.3f)  menor saida ativo=%.1f%%\n",
           pl.th, peak, max_step, OUT_SLEW_PCT_S*DT, min_out_active);
    CHECK(max_step <= OUT_SLEW_PCT_S*DT + 1e-3, "saida variou mais rapido que a rampa");
    CHECK(min_out_active >= MOTOR_IDLE_PCT - 1e-3, "saida ativa abaixo da marcha lenta");
    if (check_settle) {
        CHECK(fabs(pl.th - SETPOINT_DEFAULT_DEG) < 1.0, "nao estabilizou a menos de 1 grau do alvo");
        CHECK(peak < 20.0, "passou mais de 20 graus do alvo");
    } else if (fabs(pl.th - SETPOINT_DEFAULT_DEG) >= 1.0 || peak >= 20.0) {
        printf("  aviso: com este braco os ganhos atuais oscilam (so informativo)\n");
    }
}

/* Giroscopio com ruido de vibracao: a rampa da saida e igual para subir e para
 * descer, entao o ruido nao pode puxar a saida media para baixo. */
static void noise_test(void) {
    Plant pl = { -38, 0, 0, 5490.0, 0.183, 0.058, 1.0, -38 };
    Control_t c; CtrlParams_t p = { KP_DEFAULT, KI_DEFAULT, KD_DEFAULT, SETPOINT_DEFAULT_DEG, 1, 1 };
    int n = (int)(20.0 / DT), i; double sum = 0, act = 0; int cnt = 0; float prev = 0;
    Control_Init(&c);
    srand(1);
    printf("== giroscopio com ruido de +-60 graus/s\n");
    for (i = 0; i < n; i++) {
        double noise = 120.0 * rand() / RAND_MAX - 60.0;
        float out = Control_Step(&c, &p, (float)pl.th, (float)(pl.w + noise), DT);
        plant_step(&pl, out);
        if (i >= n / 2) { sum += pl.th; act += fabs(out - prev); cnt++; }
        prev = out;
    }
    printf("  angulo medio na segunda metade: %.2f   pulsacao da saida (media |dOut| por ciclo): %.3f %%\n", sum / cnt, act / cnt);
    CHECK(fabs(sum / cnt) < 1.5, "o ruido do giroscopio tirou o braco do alvo");
}

static void arm_test(void) {
    Control_t c; CtrlParams_t p = { KP_DEFAULT, KI_DEFAULT, KD_DEFAULT, 0, 1, 1 };
    float out = 0.0f; int i;
    Control_Init(&c);
    printf("== armar e desarmar\n");
    for (i = 0; i < 400; i++) out = Control_Step(&c, &p, -10.0f, 0.0f, DT);
    CHECK(out > 0.0f, "deveria estar atuando");
    p.armed = 0;
    out = Control_Step(&c, &p, 0.0f, 0.0f, DT);
    CHECK(out == 0.0f && !c.active, "desarmado a saida tem que ser 0");
    p.armed = 1;
    out = Control_Step(&c, &p, -10.0f, 0.0f, DT);
    CHECK(c.active && out <= OUT_SLEW_PCT_S*DT + 1e-3, "ao rearmar a saida parte de 0 em rampa");
    out = Control_Step(&c, &p, 170.0f, 0.0f, DT);
    CHECK(out >= 0.0f && out <= OUT_LIMIT_PCT, "sem corte por angulo: a saida segue o PID");
}

static void filter_test(void) {
    float a = 179.0f; int i;
    printf("== filtro e wrap\n");
    CHECK(fabsf(Angle_Wrap180(190.0f) + 170.0f) < 1e-3, "wrap de 190");
    CHECK(fabsf(Angle_Wrap180(-190.0f) - 170.0f) < 1e-3, "wrap de -190");
    CHECK(fabsf(Angle_Wrap180(356.14f) + 3.86f) < 1e-2, "wrap de 356");
    for (i = 0; i < 6000; i++) a = Angle_FilterStep(a, -179.0f, 0.0f, DT, COMP_TAU_S, 0.0f);   /* cruza +-180 pelo lado curto */
    CHECK(fabsf(a + 179.0f) < 0.1f, "filtro cruzando +-180");
    a = -38.0f;
    for (i = 0; i < 6000; i++) a = Angle_FilterStep(a, 0.0f, 0.0f, DT, COMP_TAU_S, 0.0f);
    CHECK(fabsf(a) < 0.1f, "filtro converge para o acelerometro");
    a = 0.0f;
    for (i = 0; i < 200; i++) a = Angle_FilterStep(a, (i+1)*10.0f*DT, 10.0f, DT, COMP_TAU_S, 0.0f);   /* gira a 10 graus/s */
    CHECK(fabsf(a - 10.0f) < 0.1f, "filtro acompanha giro constante");
    /* Caso real medido: com o motor forte o acelerometro marcava -40 enquanto o
     * braco subia de -38 ate +40. Com a leitura rejeitada, vale so o giroscopio. */
    a = -38.0f;
    for (i = 0; i < 560; i++) a = Angle_FilterStep(a, -40.0f, 28.0f, DT, 0.0f, 0.0f);   /* 2,8 s a 28 graus/s */
    CHECK(fabsf(a - 40.4f) < 0.5f, "com o acelerometro rejeitado o angulo segue o giroscopio");
    /* Em voo: correcao lenta segura a deriva do giroscopio (bias 0,07 graus/s por 190 s = 13 graus) */
    a = 0.0f;
    for (i = 0; i < (int)(190.0f / DT); i++) a = Angle_FilterStep(a, 0.0f, 0.07f, DT, COMP_TAU_FLIGHT_S, ACC_FLIGHT_GATE_DEG);
    CHECK(fabsf(a) < 3.0f, "a correcao lenta nao segurou a deriva do giroscopio");
    a = 0.0f;
    for (i = 0; i < 400; i++) a = Angle_FilterStep(a, 40.0f, 0.0f, DT, COMP_TAU_FLIGHT_S, ACC_FLIGHT_GATE_DEG);
    CHECK(fabsf(a) < 1e-3f, "leitura absurda do acelerometro (40 graus de diferenca) deveria ser descartada em voo");
}

int main(void) {
    filter_test();
    arm_test();
    noise_test();
    /* G = (graus/s2 por 1 % de PWM) x (% em que paira): com 300 e 18,3 % da 5490 */
    run("braco medido",                  5490.0, 0.183, 0.058, 1, 1);
    run("braco mais fraco e mais lento", 2200.0, 0.183, 0.075, 0, 1);
    run("braco mais forte e mais lento", 6950.0, 0.183, 0.075, 0, 1);
    /* Pairando com mais PWM, o integrador demora a voltar depois da falha do sensor */
    run("paira com 22 %",                6600.0, 0.220, 0.058, 0, 0);
    printf(fails ? "\n%d verificacao(oes) FALHARAM\n" : "\ntodas as verificacoes passaram\n", fails);
    return fails != 0;
}
