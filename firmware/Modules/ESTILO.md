# Padrão de código e documentação

Vale para tudo em `Modules/`. O código gerado pelo CubeMX (`Core/`) fica como está.

## Documentação: Doxygen

O MISRA C não define formato de documentação; o padrão de fato em C embarcado é o **Doxygen**.
O `Doxyfile` na raiz já está configurado (`doxygen Doxyfile` gera `docs/html`).

- Todo arquivo começa com `@file` e `@brief`; algoritmos levam `@details`, a fórmula e `@par Referencias`.
- Funções públicas são documentadas no `.h` (`@brief`, `@param[in]`/`[out]`, `@return`). Funções `static` são documentadas no `.c`.
- Macros e constantes: `/** ... */` com unidade (graus, %, ms) e o porquê do valor.
- Tudo em português. Sem acentos, como o resto do código.
- Código que não é nosso (fórmula, registrador, protocolo) leva a **referência** no `@par Referencias`: datasheet, nota de aplicação, livro ou site, com o documento e a revisão.

### Regra MISRA 3.1 e URLs

`/*` e `//` não podem aparecer dentro de comentário. Por isso URLs vão **sem** `https://`
(ex.: `www.ti.com/product/DRV8871/datasheet`). O Doxygen reconhece endereços `www.` sozinhos.

## Nomes

| Item | Padrão | Exemplo |
|---|---|---|
| Função pública | `Modulo_Acao` | `Motor_Set`, `Pid_Step` |
| Função `static` | `Verbo` ou `Objeto_Acao`, sem prefixo de módulo | `PutFixed3`, `Led_Set` |
| Tipo | `Nome_t` | `Control_t`, `MpuSample_t` |
| Enum (valores) | `PREFIXO_VALOR` | `APP_RUNNING`, `DEBUG_READ_FAIL` |
| Macro, constante | `MAIUSCULAS_COM_UNIDADE` | `OUT_LIMIT_PCT`, `COMP_TAU_S` |
| Variável local | `minusculas_com_underscore` | `gyro_bias` |
| Variável de arquivo | `s_camelCase` | `s_params`, `s_rxByte` |
| Variável global | `g_minusculas` | `g_debug` |
| Tarefa (função) | `NomeTask` | `ControlTask` |
| Semáforo, mutex, fila | `s_<nome>Sem`, `s_<nome>Mutex`, `s_<nome>Queue` | `s_tickSem`, `s_uartMutex` |
| Nome da tarefa no RTOS | curto, minúsculo | `"control"`, `"cmd"` |
| Pilha e prioridade | `<TAREFA>_TASK_STACK_WORDS`, `<TAREFA>_TASK_PRIORITY` | `CONTROL_TASK_PRIORITY` |

Sufixos de unidade nos nomes: `_deg`, `_dps` (graus/s), `_pct`, `_s`, `_ms`, `_hz`, `_g`.

## Referências verificadas (usadas nos módulos)

- MPU6050: InvenSense, *MPU-6000 and MPU-6050 Register Map and Descriptions*, RM-MPU-6000A-00 Rev 4.2.
- DRV8871: TI, SLVSCY9B, www.ti.com/product/DRV8871/datasheet
- Ângulo pelo acelerômetro: NXP AN3461, www.nxp.jp/docs/en/application-note/AN3461.pdf
- Filtro complementar: S. Colton, *The Balance Filter*, MIT 2007, web.mit.edu/scolton/www/filter.pdf
- PID: Åström e Hägglund, *PID Controllers*, 2ª ed., 1995; Åström e Murray, *Feedback Systems*, cap. 10 (www.cds.caltech.edu/~murray/amwiki/PID_Control.html); B. Beauregard, *Improving the Beginner's PID* (www.datsi.fi.upm.es/docencia/DMC/guia_pid/improving-the-beginners-pid-derivative-kick)
- FreeRTOS: www.freertos.org/a00124.html (`xSemaphoreGiveFromISR`), www.freertos.org/a00119.html (`xQueueSendFromISR`); R. Barry, *Mastering the FreeRTOS Real Time Kernel*.
- STM32F103: ST RM0008 (timers e USART), UM1850 (HAL).
