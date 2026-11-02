/**
 * @file    serial.c
 * @brief   USART2 (PA2 = TX, PA3 = RX, 115200 8N1): comandos, telemetria e LED de estado (PC13).
 *
 * @par Comandos (terminados com Enter)
 *  - "p 2.0", "i 0.5", "d 0.1": ganhos Kp, Ki e Kd;
 *  - "s 10": setpoint em graus;
 *  - "m 1" / "m 0": liga / desliga o motor;
 *  - "?": mostra os valores atuais.
 *
 * @par Telemetria
 * A 20 Hz, so com o controle rodando: "angulo,setpoint,saida".
 *
 * @par LED PC13
 *  - piscando rapido = MPU6050 nao responde (motor solto);
 *  - aceso fixo = calibrando o giroscopio (NAO mexa no braco);
 *  - piscando 1 Hz = controle rodando.
 *
 * @par Tarefas e filas (FreeRTOS)
 *  - CommandTask (prioridade 2) le os bytes da fila s_rxQueue, alimentada pela
 *    ISR de recepcao, e monta as linhas de comando;
 *  - TelemetryTask (prioridade 1) envia a telemetria e pisca o LED;
 *  - s_uartMutex garante que so uma tarefa use o TX por vez.
 *
 * @par Referencias
 *  - Conversao de float para texto feita a mao (PutFixed3): a libc "nano" do
 *    projeto nao traz printf de float por padrao (opcao -u _printf_float,
 *    ver README.nano do newlib-nano). Implementacao propria, sem fonte externa.
 *  - FreeRTOS, xQueueSendFromISR (ISR -> tarefa): www.freertos.org/a00119.html
 *  - ST, RM0008, capitulo "Universal synchronous asynchronous receiver
 *    transmitter (USART)"; HAL: UM1850 (STM32F1 HAL e drivers LL).
 */
#include "serial.h"

#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "app.h"
#include "debug.h"
#include "usart.h"

/** Bytes que cabem na fila de recepcao. */
#define RX_QUEUE_LEN            32U
/** Tamanho maximo de uma linha de comando, com o '\0'. */
#define LINE_LEN                24U
/** Periodo da telemetria, em ms (20 Hz). */
#define TELEM_PERIOD_MS         50U
/** Voltas das tarefas entre duas medidas de folga de pilha. */
#define STACK_MARK_PERIOD       40U
/** Pilha da CommandTask e da TelemetryTask, em palavras de 4 bytes. */
#define CMD_TASK_STACK_WORDS    192U
#define TELEM_TASK_STACK_WORDS  192U   /**< ver CMD_TASK_STACK_WORDS */
/** Prioridades (maior numero = mais urgente; ambas abaixo da ControlTask). */
#define CMD_TASK_PRIORITY       (tskIDLE_PRIORITY + 2U)
#define TELEM_TASK_PRIORITY     (tskIDLE_PRIORITY + 1U)   /**< ver CMD_TASK_PRIORITY */
/** Espera maxima pelo mutex do TX e pela transmissao, em ms. */
#define UART_TX_TIMEOUT_MS      20U
/** Espera por um byte antes de conferir se a recepcao continua armada, em ms. */
#define RX_POLL_MS              200U

/** Mutex: transmissao pela USART2. */
static SemaphoreHandle_t s_uartMutex;
/** Fila de bytes: ISR da USART2 -> CommandTask. */
static QueueHandle_t     s_rxQueue;
/** Byte que a HAL esta recebendo agora (a ISR o copia para a fila). */
static uint8_t           s_rxByte;

/* ======================================================================= */
/*                       Texto (sem printf de float)                        */
/* ======================================================================= */

/* As funcoes Put... escrevem em buf (de tamanho size) a partir de pos e
 * retornam a nova posicao. O que nao couber e descartado. */

/** @brief Escreve um caractere em buf[pos]. @return nova posicao. */
static size_t PutChar(char *buf, size_t size, size_t pos, char c)
{
    size_t next = pos;

    if (pos < size) {
        buf[pos] = c;
        next     = pos + 1U;
    }
    return next;
}

/** @brief Escreve uma string terminada em '\0'. @return nova posicao. */
static size_t PutStr(char *buf, size_t size, size_t pos, const char *s)
{
    size_t p = pos;
    size_t i = 0U;

    while (s[i] != '\0') {
        p = PutChar(buf, size, p, s[i]);
        i++;
    }
    return p;
}

/** @brief Escreve v com 3 casas decimais (saturado em +-99999). @return nova posicao. */
static size_t PutFixed3(char *buf, size_t size, size_t pos, float v)
{
    static const char digits[] = "0123456789";
    char     tmp[10];
    size_t   n = 0U;
    size_t   p = pos;
    float    scaled = v;
    int32_t  x;
    uint32_t mag;
    uint32_t ip;
    uint32_t fp;

    if (scaled > 99999.0f) {
        scaled = 99999.0f;
    }
    if (scaled < -99999.0f) {
        scaled = -99999.0f;
    }
    scaled *= 1000.0f;
    scaled += (scaled >= 0.0f) ? 0.5f : -0.5f;    /* arredonda */
    x = (int32_t)scaled;
    if (x < 0) {
        p = PutChar(buf, size, p, '-');
        x = -x;
    }
    mag = (uint32_t)x;
    ip  = mag / 1000U;
    fp  = mag % 1000U;

    do {                                          /* parte inteira, de tras para a frente */
        tmp[n] = digits[ip % 10U];
        n++;
        ip /= 10U;
    } while (ip != 0U);
    while (n > 0U) {
        n--;
        p = PutChar(buf, size, p, tmp[n]);
    }
    p = PutChar(buf, size, p, '.');
    p = PutChar(buf, size, p, digits[fp / 100U]);
    p = PutChar(buf, size, p, digits[(fp / 10U) % 10U]);
    p = PutChar(buf, size, p, digits[fp % 10U]);
    return p;
}

/** @brief true se c e um digito decimal. */
static bool IsDigit(char c)
{
    return (c >= '0') && (c <= '9');
}

/** @brief Valor numerico (0 a 9) de um digito decimal. */
static float DigitValue(char c)
{
    const int32_t d = (int32_t)c - (int32_t)'0';

    return (float)d;
}

/**
 * @brief  Le um numero tipo "-12.5" (aceita virgula e espacos ou "=" antes).
 * @param[in]  s    texto.
 * @param[out] out  valor lido (so escrito se houver digitos).
 * @return false se nao houver digitos.
 */
static bool ParseFloat(const char *s, float *out)
{
    size_t i     = 0U;
    float  sign  = 1.0f;
    float  v     = 0.0f;
    float  scale = 0.1f;
    bool   any   = false;

    while ((s[i] == ' ') || (s[i] == '=')) {
        i++;
    }
    if (s[i] == '-') {
        sign = -1.0f;
        i++;
    } else if (s[i] == '+') {
        i++;
    } else {
        /* sem sinal */
    }
    while (IsDigit(s[i])) {
        v   = (v * 10.0f) + DigitValue(s[i]);
        any = true;
        i++;
    }
    if ((s[i] == '.') || (s[i] == ',')) {
        i++;
        while (IsDigit(s[i])) {
            v     += DigitValue(s[i]) * scale;
            scale *= 0.1f;
            any    = true;
            i++;
        }
    }
    if (any) {
        *out = sign * v;
    }
    return any;
}

/* ======================================================================= */
/*                              Hardware                                    */
/* ======================================================================= */

/** @brief Liga ou desliga o LED da placa (PC13, acende com nivel baixo). */
static void Led_Set(bool on)
{
    /* LED da Blue Pill acende com nivel baixo */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/**
 * @brief  Transmite buf pela USART2. Se o mutex nao vier em UART_TX_TIMEOUT_MS
 *         ou a transmissao falhar, a mensagem se perde: a serial e so um
 *         auxilio e nao pode segurar as tarefas.
 * @param[in] buf  bytes a enviar.
 * @param[in] len  quantidade de bytes.
 */
static void Uart_Send(const char *buf, size_t len)
{
    if (xSemaphoreTake(s_uartMutex, pdMS_TO_TICKS(UART_TX_TIMEOUT_MS)) == pdTRUE) {
        (void)HAL_UART_Transmit(&huart2, (const uint8_t *)buf, (uint16_t)len, UART_TX_TIMEOUT_MS);
        (void)xSemaphoreGive(s_uartMutex);
    }
}

/**
 * @brief  Arma a recepcao do proximo byte. Se a HAL recusar (UART travada por
 *         uma transmissao), a CommandTask arma de novo em ate RX_POLL_MS.
 */
static void Uart_RxArm(void)
{
    (void)HAL_UART_Receive_IT(&huart2, &s_rxByte, 1U);
}

/**
 * @brief  Liga o pull-up do RX (PA3). O CubeMX deixa o RX flutuando; sem
 *         conversor ligado ele capta o ruido do motor e vira uma chuva de
 *         bytes e erros falsos. O pull-up segura a linha em repouso (nivel alto).
 */
static void Uart_RxPullUp(void)
{
    GPIO_InitTypeDef g = { 0 };

    g.Pin  = GPIO_PIN_3;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &g);
}

/* ======================================================================= */
/*                       Interrupcoes (ISR -> tarefa)                       */
/* ======================================================================= */

/**
 * @brief  ISR: um byte chegou pela USART2. Manda para a fila e arma a
 *         recepcao de novo. Com a fila cheia o byte se perde.
 * @param[in] huart  UART que gerou a interrupcao.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart == &huart2) && (s_rxQueue != NULL)) {
        BaseType_t woken = pdFALSE;

        /* Fila cheia: o byte se perde */
        (void)xQueueSendFromISR(s_rxQueue, &s_rxByte, &woken);
        Uart_RxArm();
        portYIELD_FROM_ISR(woken);
    }
}

/** @brief ISR: erro de recepcao. Conta o erro e arma a recepcao de novo. */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2) {
        Debug_Count(DEBUG_UART_ERROR);
        Uart_RxArm();
    }
}

/* ======================================================================= */
/*                               Tarefas                                    */
/* ======================================================================= */

/** @brief Envia "# kp=.. ki=.. kd=.. sp=.. motor=.." com os valores atuais. */
static void SendParams(void)
{
    char         buf[80];
    size_t       n = 0U;
    CtrlParams_t p;

    App_GetParams(&p);

    n = PutStr(buf, sizeof(buf), n, "# kp=");
    n = PutFixed3(buf, sizeof(buf), n, p.kp);
    n = PutStr(buf, sizeof(buf), n, " ki=");
    n = PutFixed3(buf, sizeof(buf), n, p.ki);
    n = PutStr(buf, sizeof(buf), n, " kd=");
    n = PutFixed3(buf, sizeof(buf), n, p.kd);
    n = PutStr(buf, sizeof(buf), n, " sp=");
    n = PutFixed3(buf, sizeof(buf), n, p.setpoint_deg);
    n = PutStr(buf, sizeof(buf), n, p.armed ? " motor=1\r\n" : " motor=0\r\n");
    Uart_Send(buf, n);
}

/**
 * @brief  Aplica uma linha de comando ja terminada em '\0' e responde com os valores atuais.
 * @param[in] line  texto do comando, p. ex. "p 2.5".
 */
static void RunCommand(const char *line)
{
    float v = 0.0f;

    /* A letra do comando so existe aqui: o resto do firmware nao conhece o
     * protocolo da serial. "?" e letras desconhecidas so mostram os valores. */
    if (ParseFloat(&line[1], &v)) {
        switch (line[0]) {
            case 'p':
            case 'P':
                (void)App_SetGain(APP_GAIN_KP, v);   /* a resposta abaixo mostra se mudou */
                break;
            case 'i':
            case 'I':
                (void)App_SetGain(APP_GAIN_KI, v);
                break;
            case 'd':
            case 'D':
                (void)App_SetGain(APP_GAIN_KD, v);
                break;
            case 's':
            case 'S':
                App_SetSetpoint(v);
                break;
            case 'm':
            case 'M':
                App_SetArmed(v != 0.0f);
                break;
            default:
                /* comando desconhecido */
                break;
        }
    }
    SendParams();
}

/**
 * @brief  Tarefa de comandos: monta as linhas que chegam pela serial e aplica os comandos.
 * @param[in] arg  nao usado.
 */
static void CommandTask(void *arg)
{
    char     line[LINE_LEN];
    size_t   len    = 0U;
    uint8_t  c      = 0U;
    uint32_t cycles = 0U;
    (void)arg;

    Uart_RxArm();

    for (;;) {
        if (xQueueReceive(s_rxQueue, &c, pdMS_TO_TICKS(RX_POLL_MS)) != pdTRUE) {
            /* Sem bytes: confere se a recepcao continua armada */
            if (huart2.RxState == HAL_UART_STATE_READY) {
                Uart_RxArm();
            }
        } else if ((c != (uint8_t)'\r') && (c != (uint8_t)'\n')) {
            if (len < (LINE_LEN - 1U)) {
                line[len] = (char)c;
                len++;
            }
        } else if (len > 0U) {
            line[len] = '\0';
            RunCommand(line);
            len = 0U;
        } else {
            /* linha vazia */
        }

        cycles++;
        if ((cycles % STACK_MARK_PERIOD) == 0U) {
            Debug_StackMark(DEBUG_TASK_CMD);
        }
    }
}

/**
 * @brief  Tarefa de telemetria: envia "angulo,setpoint,saida" a 20 Hz e pisca o LED conforme o estado.
 * @param[in] arg  nao usado.
 */
static void TelemetryTask(void *arg)
{
    char        buf[48];
    Telemetry_t t;
    uint32_t    count = 0U;
    TickType_t  last  = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        AppState_t state;

        vTaskDelayUntil(&last, pdMS_TO_TICKS(TELEM_PERIOD_MS));
        count++;
        if ((count % STACK_MARK_PERIOD) == 0U) {
            Debug_StackMark(DEBUG_TASK_TELEM);
        }

        state = App_GetState();
        switch (state) {
            case APP_NO_IMU:
                Led_Set((count & 1U) != 0U);       /* 10 Hz */
                break;
            case APP_CALIBRATING:
                Led_Set(true);
                break;
            default:
                Led_Set((count % 20U) < 10U);      /* 1 Hz */
                break;
        }

        if (state == APP_RUNNING) {
            size_t n = 0U;

            App_GetTelemetry(&t);
            n = PutFixed3(buf, sizeof(buf), n, t.angle_deg);
            n = PutChar(buf, sizeof(buf), n, ',');
            n = PutFixed3(buf, sizeof(buf), n, t.setpoint_deg);
            n = PutChar(buf, sizeof(buf), n, ',');
            n = PutFixed3(buf, sizeof(buf), n, t.output_pct);
            n = PutStr(buf, sizeof(buf), n, "\r\n");
            Uart_Send(buf, n);
        }
    }
}

/* ======================================================================= */
/*                            Inicializacao                                 */
/* ======================================================================= */

bool Serial_Init(void)
{
    bool ok;

    Uart_RxPullUp();

    s_uartMutex = xSemaphoreCreateMutex();
    s_rxQueue   = xQueueCreate(RX_QUEUE_LEN, sizeof(uint8_t));
    ok = (s_uartMutex != NULL) && (s_rxQueue != NULL);

    /* Tamanho da pilha em palavras de 4 bytes */
    if (ok) {
        ok = xTaskCreate(CommandTask, "cmd", CMD_TASK_STACK_WORDS, NULL,
                         CMD_TASK_PRIORITY, NULL) == pdPASS;
    }
    if (ok) {
        ok = xTaskCreate(TelemetryTask, "telem", TELEM_TASK_STACK_WORDS, NULL,
                         TELEM_TASK_PRIORITY, NULL) == pdPASS;
    }
    return ok;
}
