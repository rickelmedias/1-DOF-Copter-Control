# Dados da bancada

Todos os dados foram coletados em 02/11/2026 lendo a RAM do STM32 pelo ST-Link com `firmware/tools/dbg_read.py`, sem parar o firmware.

| Pasta | Conteúdo |
|---|---|
| `tests/` | Refino do PID, regime: 5 conjuntos de ganhos (T1 a T5), N = 1200 amostras (60 s a 20 Hz) cada, depois de 20 s de estabilização. `metricas.csv` tem as métricas calculadas. |
| `steps/` | Refino do PID, degraus de setpoint (+10°, −20° e volta a 0,7°) com os mesmos 5 conjuntos (D1 a D5). `*_eventos.txt` marca o instante de cada degrau. |
| `validation/` | Validação do firmware final (Ki = 0,08): regime (`V1`) e degraus (`V2`). |
| `raw_readings/` | Saídas de `dbg_read.py --log` usadas nas figuras do relatório (partida, haste pairando, tempos, histogramas). |

Colunas dos CSV: `indice` (amostra do registro), `t_s`, `angulo_graus`, `velocidade_dps`, `ang_acel_graus` (ângulo só pelo acelerômetro), `acel_g`, `saida_pct`, e os ganhos usados (`kp`, `ki`, `kd`, `aw`).

As leituras são feitas em blocos de 16 s e costuradas pelo índice da amostra; as 60 amostras mais antigas de cada leitura são descartadas porque a leitura da RAM não é atômica.
