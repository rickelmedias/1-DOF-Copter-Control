/**
 * @file    mpu6050.h
 * @brief   Driver do MPU6050 pelo I2C1 (PB6 = SCL, PB7 = SDA), sem biblioteca.
 *
 * @details Configuracao escolhida (registradores do mapa da InvenSense):
 *  - PWR_MGMT_1 (0x6B) = 0x01: sai do sleep, clock = PLL do giroscopio X;
 *  - CONFIG (0x1A) = 0x03: filtro digital (DLPF) de ~44 Hz, taxa interna 1 kHz;
 *  - SMPLRT_DIV (0x19) = 4: taxa de amostragem = 1 kHz / (1 + 4) = 200 Hz;
 *  - GYRO_CONFIG (0x1B) = 0x10: +-1000 graus/s, 32,8 LSB por grau/s;
 *  - ACCEL_CONFIG (0x1C) = 0x18: +-16 g, 2048 LSB por g.
 * Uma leitura em rajada de 14 bytes a partir de ACCEL_XOUT_H (0x3B) traz
 * acelerometro (6), temperatura (2) e giroscopio (6), cada eixo em 16 bits,
 * byte mais significativo primeiro.
 *
 * @par Referencias
 *  - InvenSense, "MPU-6000 and MPU-6050 Register Map and Descriptions",
 *    RM-MPU-6000A-00 Rev 4.2 (registradores 25 a 28, 59 a 72 e 107).
 *  - InvenSense, "MPU-6000 and MPU-6050 Product Specification",
 *    PS-MPU-6000A-00 (sensibilidades e endereco I2C 0x68 com AD0 em GND).
 *  - Angulo pelo acelerometro: NXP AN3461, Rev. 6, 2013:
 *    www.nxp.jp/docs/en/application-note/AN3461.pdf
 */
#ifndef MPU6050_H
#define MPU6050_H

#include <stdbool.h>

/** Uma leitura do sensor, ja convertida para unidades fisicas. */
typedef struct {
    float acc_angle_deg;   /**< angulo do acelerometro, graus, ja com ANGLE_SIGN e ANGLE_OFFSET_DEG */
    float acc_x_g;         /**< componente da aceleracao que entra no angulo, em g (diagnostico)    */
    float acc_z_g;         /**< componente Z da aceleracao, em g (diagnostico)                      */
    float gyro_dps;        /**< velocidade do giroscopio, graus/s, ainda com o bias                 */
} MpuSample_t;

/**
 * @brief  Acorda e configura o sensor. Usa vTaskDelay: chamar de dentro de uma tarefa.
 * @return true se todas as escritas I2C deram certo.
 */
bool Mpu_Init(void);

/**
 * @brief  Le acelerometro e giroscopio em uma unica transferencia I2C.
 * @param[out] s  leitura convertida (so e escrita se o retorno for true).
 * @return false se o I2C falhou ou se os tres eixos do acelerometro vieram
 *         zerados (sensor reiniciado por queda de tensao).
 */
bool Mpu_Read(MpuSample_t *s);

/**
 * @brief  Reinicia o periferico I2C1 (o I2C do STM32F1 pode travar em BUSY
 *         depois de um ruido; ver erratas do STM32F103).
 */
void Mpu_BusRecover(void);

#endif /* MPU6050_H */
