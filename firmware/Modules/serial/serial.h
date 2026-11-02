/**
 * @file    serial.h
 * @brief   USART2 (PA2 = TX, PA3 = RX, 115200 8N1): comandos, telemetria e LED de estado.
 *
 * @details Protocolo e tarefas descritos em serial.c.
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>

/**
 * @brief  Cria o mutex da UART, a fila de recepcao e as tarefas de comando e de telemetria.
 * @return false se faltou memoria no heap do FreeRTOS.
 */
bool Serial_Init(void);

#endif /* SERIAL_H */
