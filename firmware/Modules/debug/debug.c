/**
 * @file    debug.c
 * @brief   Implementacao do diagnostico em RAM (ver debug.h).
 */
#include "debug.h"

#if DEBUG_ENABLE

#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "main.h"

volatile DebugData_t g_debug;

/** @brief Converte (v x scale) para inteiro de 16 bits, saturando nos extremos. */
static int16_t ToScaled(float v, float scale)
{
    float x = v * scale;

    if (x > 32767.0f) {
        x = 32767.0f;
    }
    if (x < -32768.0f) {
        x = -32768.0f;
    }
    return (int16_t)x;
}

void Debug_Count(DebugCount_t what)
{
    if ((uint32_t)what < (uint32_t)DEBUG_COUNT_N) {
        g_debug.count[what]++;
    }
}

void Debug_Sample(float angle_deg, float rate_dps, float acc_x_g, float acc_z_g, float out_pct)
{
    const float    r       = fabsf(rate_dps);
    const uint32_t samples = g_debug.samples;
    uint32_t       div;

    if (samples == 0U) {
        g_debug.angle_max = angle_deg;
        g_debug.angle_min = angle_deg;
        g_debug.log_div   = (uint16_t)DEBUG_LOG_DIV;
        g_debug.log_size  = (uint16_t)DEBUG_LOG_N;
    }
    if (angle_deg > g_debug.angle_max) {
        g_debug.angle_max = angle_deg;
    }
    if (angle_deg < g_debug.angle_min) {
        g_debug.angle_min = angle_deg;
    }
    if (r > g_debug.rate_max) {
        g_debug.rate_max = r;
    }

    /* A decimacao vale o que estiver na RAM: tools/dbg_read.py pode troca-la
     * com o firmware rodando (1 = 200 Hz, registro de 1,6 s). */
    div = (uint32_t)g_debug.log_div;
    if ((div != 0U) && ((samples % div) == 0U)) {
        const uint32_t count = g_debug.log_n;
        const uint32_t i     = count % DEBUG_LOG_N;

        g_debug.log[i][0] = ToScaled(angle_deg, 10.0f);
        g_debug.log[i][1] = ToScaled(rate_dps, 10.0f);
        g_debug.log[i][2] = ToScaled(acc_x_g, 1000.0f);
        g_debug.log[i][3] = ToScaled(acc_z_g, 1000.0f);
        g_debug.log[i][4] = ToScaled(out_pct, 10.0f);
        g_debug.log_n     = count + 1U;
    }
    g_debug.samples = samples + 1U;
}

void Debug_StackMark(uint32_t idx)
{
    if (idx < DEBUG_TASK_COUNT) {
        g_debug.stack_free[idx] = (uint32_t)uxTaskGetStackHighWaterMark(NULL);
    }
    g_debug.heap_free     = (uint32_t)xPortGetFreeHeapSize();
    g_debug.heap_min_free = (uint32_t)xPortGetMinimumEverFreeHeapSize();
}

/* ======================================================================= */
/*                  Tempos (contador de ciclos do DWT)                      */
/* ======================================================================= */

/** Ciclos de CPU por microssegundo (SYSCLK de 72 MHz). */
#define CYCLES_PER_US           72U
/** Um periodo de controle em ciclos: 5 ms x 72 MHz. */
#define PERIOD_CYCLES           (5000U * CYCLES_PER_US)

/** Instante (CYCCNT) da ultima interrupcao do TIM2; escrito so na ISR. */
static volatile uint32_t s_lastTickCycles;
/** Instante da interrupcao que acordou o ciclo atual; escrito so na tarefa de controle. */
static uint32_t          s_cycleTickCycles;
/** Instante em que a tarefa de controle acordou no ciclo atual. */
static uint32_t          s_cycleBeginCycles;

void Debug_TimingInit(void)
{
    /* DWT_CYCCNT: contador de ciclos do nucleo (ARM DDI 0337, Cortex-M3 TRM; CoreSight DWT). */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT       = 0U;
    DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
}

void Debug_TimingTick(void)
{
    const uint32_t now = DWT->CYCCNT;
    const uint32_t n   = g_debug.timing_ticks + 1U;

    if (n >= 2U) {
        const uint32_t period = now - s_lastTickCycles;

        if ((g_debug.period_min == 0U) || (period < g_debug.period_min)) {
            g_debug.period_min = period;
        }
        if (period > g_debug.period_max) {
            g_debug.period_max = period;
        }
        {
            const uint32_t us  = period / CYCLES_PER_US;
            uint32_t       bin = (us > 4990U) ? (us - 4990U) : 0U;

            if (bin >= DEBUG_HIST_N) {
                bin = DEBUG_HIST_N - 1U;
            }
            g_debug.hist_period[bin]++;
        }
    }
    s_lastTickCycles       = now;
    g_debug.timing_ticks   = n;
}

void Debug_TimingBegin(void)
{
    s_cycleBeginCycles = DWT->CYCCNT;
    s_cycleTickCycles  = s_lastTickCycles;
    g_debug.timing_takes++;
}

void Debug_TimingEnd(void)
{
    const uint32_t now     = DWT->CYCCNT;
    const uint32_t wake    = s_cycleBeginCycles - s_cycleTickCycles;
    const uint32_t exec    = now - s_cycleBeginCycles;
    const uint32_t latency = now - s_cycleTickCycles;

    if (wake > g_debug.wake_max) {
        g_debug.wake_max = wake;
    }
    if (exec > g_debug.exec_max) {
        g_debug.exec_max = exec;
    }
    if (latency > g_debug.latency_max) {
        g_debug.latency_max = latency;
    }
    if (latency > PERIOD_CYCLES) {
        g_debug.deadline_miss++;
    }
    g_debug.latency_sum_us += latency / CYCLES_PER_US;
    g_debug.latency_n++;
    {
        uint32_t bin = (latency / CYCLES_PER_US) / 50U;

        if (bin >= DEBUG_HIST_N) {
            bin = DEBUG_HIST_N - 1U;
        }
        g_debug.hist_latency[bin]++;
    }
}

void Debug_Calibration(float gyro_bias_dps, float start_angle_deg)
{
    g_debug.gyro_bias_dps   = gyro_bias_dps;
    g_debug.start_angle_deg = start_angle_deg;
}

void Debug_CutoffTrip(float angle_deg)
{
    g_debug.cutoff_angle_deg = angle_deg;
    g_debug.cutoff_count++;
}

#endif /* DEBUG_ENABLE */
