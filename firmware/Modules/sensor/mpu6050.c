/**
 * @file    mpu6050.c
 * @brief   Implementacao do driver do MPU6050 (ver mpu6050.h).
 */
#include "mpu6050.h"

#include <math.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "angle_filter.h"
#include "app_config.h"
#include "i2c.h"

#define MPU_ADDR              0xD0U          /* 0x68 << 1: AD0 em GND. Se AD0 em 3V3 use 0xD2 */
#define MPU_REG_SMPLRT_DIV    0x19U
#define MPU_REG_CONFIG        0x1AU
#define MPU_REG_GYRO_CONFIG   0x1BU
#define MPU_REG_ACCEL_CONFIG  0x1CU
#define MPU_REG_ACCEL_XOUT_H  0x3BU
#define MPU_REG_PWR_MGMT_1    0x6BU
#define MPU_FRAME_LEN         14U            /* acel (6) + temperatura (2) + giro (6) */
#define MPU_GYRO_LSB_PER_DPS  32.8f          /* fundo de escala +-1000 graus/s */
#define MPU_ACC_LSB_PER_G     2048.0f        /* fundo de escala +-16 g */
#define RAD_TO_DEG            57.29578f

/** @brief Escreve um byte em um registrador do MPU6050 (timeout de 10 ms). */
static bool Mpu_Write(uint8_t reg, uint8_t val)
{
    uint8_t data = val;

    return HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             &data, 1U, 10U) == HAL_OK;
}

/** @brief Junta dois bytes (mais significativo primeiro) num inteiro com sinal. */
static int16_t Mpu_Be16(uint8_t hi, uint8_t lo)
{
    const uint16_t u = (uint16_t)(((uint32_t)hi << 8U) | (uint32_t)lo);

    return (int16_t)u;
}

bool Mpu_Init(void)
{
    bool ok = Mpu_Write(MPU_REG_PWR_MGMT_1, 0x01U);       /* acorda, clock = PLL do giro X */

    if (ok) {
        vTaskDelay(pdMS_TO_TICKS(50U));
        ok = Mpu_Write(MPU_REG_CONFIG, 0x03U);            /* filtro digital ~44 Hz */
    }
    if (ok) {
        ok = Mpu_Write(MPU_REG_SMPLRT_DIV, 0x04U);        /* 1 kHz / (1+4) = 200 Hz */
    }
    /* Escalas largas: a queda do braco passa de 500 graus/s e a vibracao da
     * helice passa de 4 g. Saturar qualquer um dos dois estraga o angulo. */
    if (ok) {
        ok = Mpu_Write(MPU_REG_GYRO_CONFIG, 0x10U);       /* +-1000 graus/s */
    }
    if (ok) {
        ok = Mpu_Write(MPU_REG_ACCEL_CONFIG, 0x18U);      /* +-16 g */
    }
    if (ok) {
        vTaskDelay(pdMS_TO_TICKS(50U));
    }
    return ok;
}

void Mpu_BusRecover(void)
{
    /* Se algum passo falhar, o proximo Mpu_Init falha e quem chamou tenta de novo */
    (void)HAL_I2C_DeInit(&hi2c1);
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();
    (void)HAL_I2C_Init(&hi2c1);
}

bool Mpu_Read(MpuSample_t *s)
{
    uint8_t b[MPU_FRAME_LEN] = { 0 };
    bool    ok;

    ok = HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, MPU_REG_ACCEL_XOUT_H,
                          I2C_MEMADD_SIZE_8BIT, b, MPU_FRAME_LEN, 3U) == HAL_OK;
    if (ok) {
        const int16_t ax = Mpu_Be16(b[0], b[1]);
        const int16_t ay = Mpu_Be16(b[2], b[3]);
        const int16_t az = Mpu_Be16(b[4], b[5]);

        /* Tudo zero = sensor dormindo/reiniciado (ex.: queda de tensao) */
        ok = (ax != 0) || (ay != 0) || (az != 0);
        if (ok) {
            const float fz = (float)az;
#if IMU_AXIS_X
            const float acc_x    = (float)ay;
            const float gyro_raw = (float)Mpu_Be16(b[8], b[9]);
#else
            const float acc_x    = -(float)ax;
            const float gyro_raw = (float)Mpu_Be16(b[10], b[11]);
#endif
            const float acc_rad  = atan2f(acc_x, fz);

            s->acc_x_g       = acc_x / MPU_ACC_LSB_PER_G;
            s->acc_z_g       = fz / MPU_ACC_LSB_PER_G;
            s->acc_angle_deg = Angle_Wrap180((ANGLE_SIGN * acc_rad * RAD_TO_DEG)
                                             + ANGLE_OFFSET_DEG);
            s->gyro_dps      = (ANGLE_SIGN * gyro_raw) / MPU_GYRO_LSB_PER_DPS;
        }
    }
    return ok;
}
