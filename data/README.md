# Dados da bancada

Coletados em 02/11/2026, lendo a RAM do STM32 pelo ST-Link com `firmware/tools/dbg_read.py`, sem parar o firmware.

- `tests/`: regime com 5 conjuntos de ganhos (T1 a T5), 60 s a 20 Hz cada, após 20 s de estabilização. `metricas.csv` traz as métricas.
- `steps/`: degraus de *setpoint* com os mesmos ganhos (D1 a D5). `*_eventos.txt` marca cada degrau.
- `validation/`: firmware final (Ki = 0,08), regime (`V1`) e degraus (`V2`).
- `raw_readings/`: saídas de `dbg_read.py --log` usadas nas figuras.

Colunas dos CSV: `indice`, `t_s`, `angulo_graus`, `velocidade_dps`, `ang_acel_graus`, `acel_g`, `saida_pct`, `kp`, `ki`, `kd`, `aw`.

A leitura é feita em blocos de 16 s, costurados pelo índice; as 60 amostras mais antigas de cada bloco são descartadas porque a leitura da RAM não é atômica.
