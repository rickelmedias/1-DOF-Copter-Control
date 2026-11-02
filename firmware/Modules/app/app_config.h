/**
 * @file    app_config.h
 * @brief   Todos os ajustes do projeto 1DOF em um lugar so.
 *
 * @details Hardware: Blue Pill STM32F103C8 + MPU6050 + DRV8871 + motor de
 *          drone com helice. Para trocar um valor, edite aqui e recompile.
 *
 * @par Referencias
 *  - DRV8871 datasheet (TI, SLVSCY9B): www.ti.com/product/DRV8871/datasheet
 *  - MPU-6000/MPU-6050 Register Map (InvenSense, RM-MPU-6000A-00 Rev 4.2)
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/** @name Amostragem */
/**@{*/

/** Periodo do controle, em s. Tem que bater com o TIM2 (PSC = 7199, ARR = 49 -> 200 Hz). */
#define CTRL_PERIOD_S           0.005f

/**@}*/

/** @name Sensor */
/**@{*/

/**
 * Eixo de rotacao do braco, visto pelo MPU6050:
 *  - 1 = gira em torno do eixo X do sensor (angulo = atan2(ay, az), giro X);
 *  - 0 = gira em torno do eixo Y do sensor (angulo = atan2(-ax, az), giro Y).
 */
#define IMU_AXIS_X              0

/** Troque para -1.0f se o angulo estiver com o sinal invertido. */
#define ANGLE_SIGN              (+1.0f)

/** Somado ao angulo medido, em graus. Sensor de cabeca para baixo: +180 leva o zero para perto de 0. */
#define ANGLE_OFFSET_DEG        180.0f

/**
 * Constante de tempo do filtro complementar, em s (maior = confia mais no
 * giroscopio). Vale com o motor parado; em voo vale COMP_TAU_FLIGHT_S.
 *
 * Com o motor ligado ele le errado: em 06/10/2026 o braco pairava perto de
 * +24 graus e o acelerometro marcava ~+5 (mediana), com amostras de -48 a +46.
 * O giroscopio continuou certo (com o motor parado os dois andam juntos, e a
 * queda medida por ele bate com onde o braco parou). Por isso, em voo, o
 * angulo e so a integral do giroscopio a partir do angulo de repouso; o erro
 * que se acumula e o desvio do giroscopio vezes o tempo de voo.
 */
#define COMP_TAU_S              2.0f

/**
 * Constante de tempo do filtro com o motor LIGADO, em s. Em voo o acelerometro
 * le errado (vibracao da helice) mas nao deriva; o giroscopio le certo mas o
 * vies residual (cerca de 0,07 graus/s) faz o angulo derivar uns 4 graus por
 * minuto (medido em 02/11/2026: acelerometro -1,9 graus contra 11,6 do
 * firmware depois de 190 s). Uma correcao bem lenta pelo acelerometro
 * (30 s) segura a deriva sem deixar o ruido passar: o ruido de 8 graus
 * vira cerca de 0,07 graus na estimativa. A polarizacao do acelerometro em
 * voo (~+6 graus a 20 %) fica como erro fixo, que se acerta com o setpoint.
 * 0 = sem correcao em voo (so giroscopio, volta a derivar).
 */
#define COMP_TAU_FLIGHT_S       30.0f

/**
 * Em voo, descarta a leitura do acelerometro que difira da estimativa por mais
 * que isto, em graus: as amostras de -48 a +46 graus vistas em voo nao entram.
 */
#define ACC_FLIGHT_GATE_DEG     25.0f

/**@}*/

/** @name Motor */
/**@{*/

/**
 * Entrada do DRV8871 que aciona o motor: (+1) = IN1 (PA8), (-1) = IN2 (PA9).
 * A helice so empurra para um lado, entao o motor nunca e invertido. Tem que
 * ser o sentido em que a helice empurra o braco para cima.
 */
#define MOTOR_SIGN              (-1)

/**
 * Frequencia do PWM, em Hz. O motor coreless 0720 quase nao tem indutancia
 * (~7 uH para ~2 ohm: constante de tempo de ~3 us, estimada pelos dados da
 * maxon para coreless de 8 mm). O periodo do PWM tem que ser curto perto disso,
 * senao a corrente acompanha cada pulso de 12 V em vez de ficar lisa:
 *  - 1 kHz e 20 kHz: a corrente chega a ~4,7 A a cada pulso e o DRV8871
 *    desarma (IOCP de 3,7 A a 6,4 A, religa em 3 ms: o "piii"). Visto na bancada;
 *  - 100 kHz: funciona, mas sem indutor a corrente ainda ondula ~2 A e o motor
 *    esquenta umas 5 vezes mais do que em corrente continua.
 * O conserto e um indutor de ~100 uH em serie com o motor (hardware/PLACA_MOTOR.md).
 * Contas em tools/pwm_heat.py. Limites do DRV8871: ate 200 kHz e pulso minimo
 * de 800 ns. O contador do TIM1 tem 16 bits: o Motor_Init calcula o prescaler.
 */
/* 200 kHz foi testado em 06/10/2026 e o braco nao equilibrou (subia e descia):
 * os pulsos ficam curtos demais para o DRV8871 (0,8 a 1,6 us, com 0,7 us de atraso). */
#define MOTOR_PWM_HZ            100000U

/**
 * 0 = PWM com roda livre (coast); 1 = PWM com freio (slow decay). Com o indutor
 * em serie, o freio deixa a corrente lisa e a tensao media no motor igual a
 * saida % x VM: e o modo que a TI recomenda no datasheet do DRV8871.
 */
#define MOTOR_SLOW_DECAY        1

/**
 * Limite da saida, em % do PWM. Com slow decay e VM = 12 V: 32 % = ~3,8 V medios
 * no motor, perto dos 3,7 V nominais. A 42 % o motor esquentou. O braco paira
 * com ~25 % (medido no registro); se nao conseguir subir, aumente aos poucos.
 */
#define OUT_LIMIT_PCT           32.0f

/** Marcha lenta, em %: com o controle ativo a saida nunca fica abaixo disso, e a helice nao para. */
#define MOTOR_IDLE_PCT          10.0f

/**
 * Variacao maxima da saida, em % por segundo, para subir e para descer. Protege
 * o driver de degraus de corrente. Menor = mais suave, porem atrasa o controle.
 * Cortar o motor (saida 0) e sempre imediato.
 */
#define OUT_SLEW_PCT_S          300.0f

/**@}*/

/** @name Controle */
/**@{*/

/**
 * Ganhos iniciais do PID (saida em % de PWM, erro em graus), ajustaveis pela
 * serial ou por tools/dbg_read.py --set.
 *
 * Calculados a partir do braco medido em 06/10/2026 (dois registros):
 *  - Kp 1,0 / Kd 0,15: oscilou de -10 a +5 graus a cada 0,45 s, saida de 10 a 32 %;
 *  - Kp 0,30 / Kd 0,09: oscilou +-3,7 graus a cada 0,35 s, saida de 13 a 24 %.
 * Do segundo sai o braco perto do equilibrio: ~300 graus/s2 de aceleracao por
 * 1 % de PWM e ~0,06 s de atraso entre a saida e o empuxo (a helice demora a
 * mudar de rotacao). Com esse atraso, ganho alto vira oscilacao: o modelo da
 * margem de fase de 4 graus para Kp 0,30 / Kd 0,09 (oscilacao prevista em
 * 18,4 rad/s; medida: 18 rad/s). Os valores abaixo dao ~43 graus de margem no
 * braco medido e pelo menos 33 graus com o ganho entre 120 e 380 e o atraso
 * entre 0,045 e 0,075 s. Contas: tools/arm_margins.py e tools/arm_tune.py.
 *
 * Refino de 02/11/2026: Ki subiu de 0,05 para 0,08 (mesma margem de fase, 32,5 graus no pior
 * caso). Em 5 conjuntos testados na bancada, com degraus de setpoint de +10, -20 graus e volta,
 * foi o de menor erro acumulado (IAE 94,6 contra 132,0 com Ki 0,05); em regime todos ficaram
 * com RMS de 0,29 a 0,32 graus. Dados em coletas/testes e coletas/degraus.
 */
#define KP_DEFAULT              0.06f   /**< % por grau             */
#define KI_DEFAULT              0.08f   /**< % por (grau * segundo) */
#define KD_DEFAULT              0.04f   /**< % por (grau / segundo) */

/**
 * Constante de tempo, em s, do filtro passa-baixa da velocidade usada no
 * termo D. Cada 0,01 s aqui custa ~10 graus de margem de fase na frequencia
 * em que o braco oscila; com 0,02 s a margem ficava curta. Com Kd baixo, o
 * ruido do giroscopio que passa para a saida e pequeno (~1 % de PWM).
 */
#define RATE_FILTER_TAU_S       0.005f

/**
 * Valor com que o integrador comeca quando o controle entra em acao, em % de
 * PWM: um pouco abaixo do que o braco precisa para pairar (medido: 18 a 23 %).
 * Sem isso, com ganhos baixos, o braco levaria mais de 8 s para sair do repouso.
 * Comecar abaixo e de proposito: comecar acima faz o braco passar do alvo.
 */
#define OUT_HOVER_PCT           16.0f

/** Angulo que o braco deve manter, em graus. */
#define SETPOINT_DEFAULT_DEG    0.7f

/**@}*/

/** @name Seguranca */
/**@{*/

/**
 * 1 = o motor liga sozinho depois da calibracao; 0 = so com o comando "m 1" na
 * serial.
 */
#define AUTO_ARM                1U

/**
 * Corte por angulo: acima de ANGLE_CUTOFF_HI_DEG ou abaixo de ANGLE_CUTOFF_LO_DEG
 * o motor e desligado e fica desligado ate "m 1" (ou reset). Acima de 90 graus a
 * gravidade deixa de puxar a haste para baixo e, com a marcha lenta, ela passa
 * por cima: aconteceu em 02/11/2026 (volta completa). Faixa normal: -37 a +15.
 */
#define ANGLE_CUTOFF_HI_DEG     60.0f
#define ANGLE_CUTOFF_LO_DEG     (-90.0f)

/**@}*/

/** @name Diagnostico */
/**@{*/

/**
 * 1 = guarda contadores e um registro circular na RAM, lidos pelo ST-Link
 * (tools/dbg_read.py, ver debug.h). Custa uma gravacao na RAM por ciclo, sem
 * bloqueio nem interrupcao. 0 = o diagnostico some do firmware.
 */
#define DEBUG_ENABLE            1U

/** Uma amostra do registro a cada DEBUG_LOG_DIV ciclos de controle (10 = 20 Hz). */
#define DEBUG_LOG_DIV           10U

/** Amostras guardadas no registro circular (320 = ultimos 16 s). */
#define DEBUG_LOG_N             320U

/**@}*/

#endif /* APP_CONFIG_H */
