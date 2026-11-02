/**
 * @file    app.c
 * @brief   Tarefa de controle do braco 1DOF (Blue Pill + MPU6050 + DRV8871 + FreeRTOS).
 *
 * @par Ligacoes
 *  - PA8  (TIM1_CH1) -> DRV8871 IN1;   PB6 (I2C1_SCL) -> MPU6050 SCL
 *  - PA9  (TIM1_CH2) -> DRV8871 IN2;   PB7 (I2C1_SDA) -> MPU6050 SDA
 *  - motor entre OUT1 e OUT2 do DRV8871
 *  - PA2 (USART2_TX) / PA3 (USART2_RX): telemetria e comandos (opcional)
 *  - PC13: LED da placa
 *
 * @par Modulos
 *  - app/     app_config.h (ajustes) e este arquivo (tarefa de controle)
 *  - sensor/  mpu6050.c (leitura) e angle_filter.c (filtro complementar)
 *  - control/ control.c (PID e rampa da saida) e pid.c
 *  - motor/   motor.c (PWM do DRV8871)
 *  - serial/  serial.c (comandos, telemetria e LED)
 *  - debug/   debug.c (diagnostico lido pelo ST-Link)
 *
 * @par Arquitetura (FreeRTOS)
 * @verbatim
 *   TIM2 (200 Hz) --ISR--> [semaforo binario s_tickSem] --> ControlTask (prioridade 3)
 *                                                            le o MPU6050, filtra o angulo,
 *                                                            Control_Step, Motor_Set
 *   USART2 RX --ISR--> [fila s_rxQueue] --> CommandTask   (prioridade 2)
 *   TelemetryTask (prioridade 1): telemetria a 20 Hz e LED de estado
 *   [s_dataMutex] protege ganhos, setpoint e telemetria entre as tarefas
 *   [s_uartMutex] garante que so uma tarefa transmite por vez
 * @endverbatim
 * Sensor, filtro, controle e motor rodam na mesma tarefa, um depois do
 * outro: a amostra lida e usada no mesmo ciclo e nenhuma etapa atrasa a outra.
 *
 * @par MISRA C:2012
 * Os modulos em Modules/ seguem as regras, conferidas com tools/misra_check.sh.
 * Desvios, todos em regras "advisory":
 *  - Dir 4.6: usamos float direto, sem um typedef float32_t;
 *  - Rule 11.4: as macros da HAL (GPIOA, GPIOC, __HAL_RCC_...) convertem
 *    enderecos em ponteiros para registradores;
 *  - Rule 8.7: funcoes chamadas pela HAL e pelo CubeMX (App_Init,
 *    App_TimerISR, HAL_UART_...Callback) precisam ser globais.
 * As macros do FreeRTOS (portYIELD_FROM_ISR) tambem geram avisos; nao sao nossas.
 * Regra 3.1: URLs nos comentarios vao sem "http" e sem barras duplas.
 *
 * @par Referencias
 *  - FreeRTOS, xSemaphoreGiveFromISR: www.freertos.org/a00124.html
 *  - Barry, R. "Mastering the FreeRTOS Real Time Kernel", cap. 6 e 7.
 *  - Padrao de nomes e de documentacao: Modules/ESTILO.md
 */
#include "app.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "angle_filter.h"
#include "app_config.h"
#include "debug.h"
#include "serial.h"
#include "motor.h"
#include "mpu6050.h"
#include "tim.h"

/** Amostras da calibracao do giroscopio: 400 ciclos de 5 ms = 2 s. */
#define CALIB_SAMPLES           400U
/** Espera maxima pelo semaforo do TIM2, em ms (4 periodos de controle). */
#define TICK_TIMEOUT_MS         20U
/** Ciclos seguidos sem amostra ate soltar o motor (10 x 5 ms = 50 ms). */
#define MAX_MISSES              10U
/** Ciclos entre duas medidas de folga de pilha. */
#define STACK_MARK_PERIOD       200U
/** Limite do comando de setpoint pela serial, em graus. */
#define SETPOINT_MAX_DEG        60.0f
/** Pilha da tarefa de controle, em palavras de 4 bytes. */
#define CONTROL_TASK_STACK_WORDS 384U
/** Prioridade da tarefa de controle: a maior do projeto (maior numero = mais urgente). */
#define CONTROL_TASK_PRIORITY   (tskIDLE_PRIORITY + 3U)

/** Semaforo binario: ISR do TIM2 -> ControlTask (um "tick" a cada 5 ms). */
static SemaphoreHandle_t s_tickSem;
/** Mutex que protege s_params e s_telem entre as tarefas. */
static SemaphoreHandle_t s_dataMutex;

/** Parametros do controle, compartilhados entre ControlTask e CommandTask (sob s_dataMutex). */
static CtrlParams_t s_params = {
    KP_DEFAULT, KI_DEFAULT, KD_DEFAULT, SETPOINT_DEFAULT_DEG, (AUTO_ARM != 0U), true
};
/** Ultimos valores do controle, para a telemetria (sob s_dataMutex). */
static Telemetry_t         s_telem;
/** Estado atual; escrito so pela ControlTask, lido pelo LED. */
static volatile AppState_t s_state = APP_NO_IMU;

/* ======================================================================= */
/*                     Acesso aos dados compartilhados                      */
/* ======================================================================= */

/** @brief Pega s_dataMutex. Com portMAX_DELAY o xSemaphoreTake so retorna depois de pegar o mutex, e o
 * xSemaphoreGive de quem pegou o mutex nao falha: os retornos nao dizem nada. */
static void Data_Lock(void)
{
    (void)xSemaphoreTake(s_dataMutex, portMAX_DELAY);
}

/** @brief Solta s_dataMutex. */
static void Data_Unlock(void)
{
    (void)xSemaphoreGive(s_dataMutex);
}

AppState_t App_GetState(void)
{
    return s_state;
}

void App_GetParams(CtrlParams_t *out)
{
    Data_Lock();
    *out = s_params;
    Data_Unlock();
}

void App_GetTelemetry(Telemetry_t *out)
{
    Data_Lock();
    *out = s_telem;
    Data_Unlock();
}

/** @brief Guarda os ultimos valores do controle para a telemetria (com mutex). */
static void App_SetTelemetry(float angle_deg, float setpoint_deg, float output_pct)
{
    Data_Lock();
    s_telem.angle_deg    = angle_deg;
    s_telem.setpoint_deg = setpoint_deg;
    s_telem.output_pct   = output_pct;
    Data_Unlock();
}

bool App_SetGain(AppGain_t gain, float value)
{
    const bool ok = (value >= 0.0f);

    if (ok) {
        Data_Lock();
        switch (gain) {
            case APP_GAIN_KP:
                s_params.kp = value;
                break;
            case APP_GAIN_KI:
                s_params.ki = value;
                break;
            default:
                s_params.kd = value;
                break;
        }
        Data_Unlock();
    }
    return ok;
}

void App_SetSetpoint(float deg)
{
    const float sp = Pid_Clamp(deg, -SETPOINT_MAX_DEG, SETPOINT_MAX_DEG);

    Data_Lock();
    s_params.setpoint_deg = sp;
    Data_Unlock();
}

void App_SetArmed(bool armed)
{
    Data_Lock();
    s_params.armed = armed;
    Data_Unlock();
}

/* ======================================================================= */
/*                       Interrupcao (ISR -> tarefa)                        */
/* ======================================================================= */

/* Chamada pelo HAL_TIM_PeriodElapsedCallback do main.c: a cada estouro do
 * TIM2 (5 ms) libera o semaforo que acorda a ControlTask (processamento
 * diferido: a ISR so sinaliza, o trabalho e feito na tarefa). */
void App_TimerISR(const TIM_HandleTypeDef *htim)
{
    if ((htim == &htim2) && (s_tickSem != NULL)) {
        BaseType_t woken = pdFALSE;

        /* Se o semaforo ja estava dado, a tarefa esta atrasada e este periodo se perde */
        Debug_TimingTick();
        (void)xSemaphoreGiveFromISR(s_tickSem, &woken);
        portYIELD_FROM_ISR(woken);
    }
}

/* ======================================================================= */
/*                                Sensor                                    */
/* ======================================================================= */

/** @brief (Re)inicia o sensor e, se ele nao responder, o I2C. So retorna quando o MPU6050 responde. */
static void Sensor_Start(void)
{
    while (!Mpu_Init()) {
        Debug_Count(DEBUG_BUS_RECOVER);
        Mpu_BusRecover();
        vTaskDelay(pdMS_TO_TICKS(200U));
    }
}

/**
 * @brief  Espera o semaforo do TIM2 (periodo fixo de 5 ms) e le o sensor.
 * @param[out] sample  leitura do sensor.
 * @return false se o semaforo nao veio ou se a leitura falhou.
 */
static bool Sensor_Sample(MpuSample_t *sample)
{
    bool ok = false;

    if (xSemaphoreTake(s_tickSem, pdMS_TO_TICKS(TICK_TIMEOUT_MS)) == pdTRUE) {
        Debug_TimingBegin();
        ok = Mpu_Read(sample);
        if (!ok) {
            Debug_Count(DEBUG_READ_FAIL);
        }
    }
    return ok;
}

/* ======================================================================= */
/*                            Inicializacao                                 */
/* ======================================================================= */

/** @brief Erro sem volta na partida (faltou heap do FreeRTOS ou um timer nao ligou):
 * LED muito rapido, motor desligado. Para o heap, aumente TOTAL_HEAP_SIZE no CubeMX. */
static void App_Fatal(void)
{
    volatile uint32_t d;

    __disable_irq();
    Motor_Set(0.0f);
    for (;;) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        for (d = 0U; d < 150000U; d++) {
            /* espera */
        }
    }
}

/* ======================================================================= */
/*                               Tarefa                                     */
/* ======================================================================= */

/**
 * @brief  Tarefa de controle: a cada 5 ms le o MPU6050, filtra o angulo,
 *         calcula a saida e aplica no motor.
 * @param[in] arg  nao usado.
 */
static void ControlTask(void *arg)
{
    Control_t    control;
    CtrlParams_t params;
    MpuSample_t  sample    = { 0.0f, 0.0f, 1.0f, 0.0f };
    float        angle;
    float        gyro_bias;
    float        sum_gyro  = 0.0f;
    float        sum_acc   = 0.0f;
    uint32_t     n         = 0U;
    uint32_t     misses    = 0U;
    uint32_t     cycles    = 0U;
    bool         motor_on  = false;  /* saida do ciclo anterior acima de zero */
    (void)arg;

    Control_Init(&control);
    Sensor_Start();                    /* sem sensor fica aqui (LED pisca rapido) */

    /* Comeca a gerar o semaforo a 200 Hz */
    Debug_TimingInit();
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) {
        App_Fatal();
    }

    /* Calibracao: braco PARADO por 2 s. Mede o bias do giro e o angulo inicial */
    s_state = APP_CALIBRATING;
    while (n < CALIB_SAMPLES) {
        if (Sensor_Sample(&sample)) {
            misses    = 0U;
            sum_gyro += sample.gyro_dps;
            sum_acc  += sample.acc_angle_deg;
            n++;
        } else {
            misses++;
            if (misses >= MAX_MISSES) {
                misses = 0U;
                Sensor_Start();
            }
        }
    }
    gyro_bias = sum_gyro / (float)CALIB_SAMPLES;
    angle     = sum_acc  / (float)CALIB_SAMPLES;
    Debug_Calibration(gyro_bias, angle);
    App_GetParams(&params);
    s_state   = APP_RUNNING;

    for (;;) {
        if (Sensor_Sample(&sample)) {
            /* Motor parado: filtro rapido (COMP_TAU_S). Motor ligado: o acelerometro so
             * corrige devagar e dentro de uma janela, para segurar a deriva do giroscopio. */
            const float tau_s       = motor_on ? COMP_TAU_FLIGHT_S : COMP_TAU_S;
            const float gate_deg    = motor_on ? ACC_FLIGHT_GATE_DEG : 0.0f;
            const float rate        = sample.gyro_dps - gyro_bias;
            float       out;

            misses = 0U;
            angle = Angle_FilterStep(angle, sample.acc_angle_deg, rate, CTRL_PERIOD_S, tau_s, gate_deg);

            App_GetParams(&params);
            if (params.armed && ((angle > ANGLE_CUTOFF_HI_DEG) || (angle < ANGLE_CUTOFF_LO_DEG))) {
                /* Corte por angulo: desliga o motor e so volta com "m 1" ou reset */
                App_SetArmed(false);
                params.armed = false;
                Debug_CutoffTrip(angle);
            }
            out = Control_Step(&control, &params, angle, rate, CTRL_PERIOD_S);
            Motor_Set(out);
            Debug_TimingEnd();
            motor_on = (out > 0.0f);

            Debug_Sample(angle, rate, sample.acc_x_g, sample.acc_z_g, out);
            App_SetTelemetry(angle, params.setpoint_deg, out);
        } else {
            /* Uma falha isolada so pula o ciclo: o motor mantem a saida.
             * 50 ms sem amostra: o sensor falhou. Motor solto ate ele voltar. */
            misses++;
            if (misses >= MAX_MISSES) {
                misses = 0U;
                Control_Stop(&control);
                Motor_Set(0.0f);
                motor_on = false;
                App_SetTelemetry(angle, params.setpoint_deg, 0.0f);
                s_state = APP_NO_IMU;
                Sensor_Start();
                s_state = APP_RUNNING;
            }
        }

        cycles++;
        if ((cycles % STACK_MARK_PERIOD) == 0U) {
            Debug_StackMark(DEBUG_TASK_CONTROL);
        }
    }
}

/* Cria os objetos do RTOS e as tarefas (documentada em app.h). Tamanho da
 * pilha em palavras de 4 bytes. */
void App_Init(void)
{
    bool ok = Motor_Init();             /* PWM do motor comeca em 0 % */

    s_tickSem   = xSemaphoreCreateBinary();
    s_dataMutex = xSemaphoreCreateMutex();
    if ((s_tickSem == NULL) || (s_dataMutex == NULL)) {
        ok = false;
    }

    /* Tamanho da pilha em palavras de 4 bytes. Prioridade maior = mais urgente */
    if (ok) {
        ok = xTaskCreate(ControlTask, "control", CONTROL_TASK_STACK_WORDS, NULL,
                         CONTROL_TASK_PRIORITY, NULL) == pdPASS;
    }
    if (ok) {
        ok = Serial_Init();
    }
    if (!ok) {
        App_Fatal();
    }
}
