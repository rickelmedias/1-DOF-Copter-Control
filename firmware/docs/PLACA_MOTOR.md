# Acionamento do motor: o que mudar no hardware

Motor: coreless 0720, 3,7 V nominal, ~2 Ω medidos. Driver atual: DRV8871 com VM = 12 V, PWM de 100 kHz.
O STM32 (Blue Pill), o MPU6050, o BEC e a fonte de 12 V **não mudam**.

## Conclusão

O motor quase não tem indutância (~7 µH; constante de tempo de ~3 µs, estimada pelos dados da maxon para
coreless de 8 mm). O período do PWM de 100 kHz é 10 µs, então a corrente **não fica lisa**: ela acompanha cada
pulso de 12 V. O empuxo depende da corrente média, mas o calor depende da corrente eficaz (RMS), que fica
umas duas vezes maior. Resultado da conta (`tools/pwm_heat.py`):

| Caso | Calor no motor | Pico de corrente |
|---|---|---|
| Hoje: DRV8871, 12 V, 100 kHz, sem indutor | ~1,5 W | ~2,1 A |
| Corrente contínua pura (o mínimo possível para o mesmo empuxo) | ~0,3 W | 0,4 A |
| **DRV8871, 12 V, 100 kHz + indutor de 100 µH** | **~0,3 W** | **~0,5 A** |
| TB6612 em 5 V, 100 kHz, sem indutor | ~0,7 W | ~1,1 A |
| TB6612 em 5 V, 20 kHz, sem indutor | ~1,9 W | ~1,3 A |
| DRV8871, 12 V, 20 kHz ou 1 kHz, sem indutor | 6 a 8 W | ~4 A (o driver desarma: "piii") |

A conta supõe 0,4 A para pairar e 7 µH no motor; nenhum dos dois foi medido. O ganho do indutor não depende
disso: com 3,5 a 30 µH no motor e 0,15 a 0,6 A para pairar, o calor com o indutor sempre cai para o valor da
corrente contínua.

## O que comprar: um indutor

**Indutor de potência de 100 µH, corrente de saturação de 2 A ou mais (melhor 3 A), resistência até ~0,3 Ω.**
É o indutor toroidal ou blindado usado em módulos LM2596 ("indutor 100uH 3A"). Entre 47 µH e 220 µH serve.

```
DRV8871 OUT2 ──[ indutor 100 µH ]── motor ── DRV8871 OUT1
```

- Em série com **um** dos fios do motor, perto do driver, **fora do braço** (pesa alguns gramas).
- O firmware fica como está: 100 kHz, modo freio, limite de 32 %.
- Com ele o pico de corrente na partida cai de ~3,4 A para ~1,5 A: o driver deixa de desarmar.

Não precisa de TB6612, MOSFET nem buck. Se um dia trocar o driver, o indutor continua necessário.

## Fiação (mau contato visto na bancada)

Em 06/10/2026 o braço ficou 8 s parado com a saída no máximo e só subiu quando o cabo foi mexido.

- **Solde** os fios do motor (são finos e a solda nos terminais racha) ou use borne bem apertado.
- Nada de protoboard no caminho do motor, do VM e do GND.
- Fio de ~22 AWG em VM, GND e saídas do driver.
- Capacitor de 470 µF + 100 nF entre VM e GND, junto ao driver (já existe um de 220 µF).

## Sensor (para o braço não ficar balançando)

- Com o motor ligado o acelerômetro marcou de −58° a +18° com o braço parado perto de 2°: é vibração.
- Prenda o MPU6050 com **fita dupla face de espuma**, o mais perto possível do **eixo de giro**, e fixe o cabo.
- Hélice sem lascas e bem encaixada (hélice torta é a maior fonte de vibração).

## Conferir depois de montar

1. Corrente na linha do VM com o braço pairando (multímetro em série). Pela conta: ~0,15 a 0,3 A hoje e
   ~0,03 a 0,15 A com o indutor.
2. Temperatura do motor depois de 2 min pairando.
3. Se ainda esquentar com o indutor, o calor é da própria carga: um **contrapeso** do outro lado do braço
   reduz o empuxo necessário (o calor cai com o quadrado da corrente).

## Teste que separa as duas causas (opcional, sem comprar nada)

Ligue o motor direto em **2 pilhas AA em série (~3 V)**, sem driver, por 1 a 2 min. É corrente contínua pura.
- Esquenta pouco: o calor vinha do PWM, e o indutor resolve.
- Esquenta igual: o calor é da carga, e o caminho é o contrapeso.

## Referências

- maxon, "PWM power stage: current ripple & motor chokes":
  support.maxongroup.com/hc/en-us/articles/360005046213
- maxon DCX 8 M (coreless de 8 mm): 13,9 µH com 4,13 Ω; resistência térmica carcaça-ambiente de 101 K/W.
- TI, DRV8871 (SLVSCY9B): IOCP de 3,7 a 6,4 A, tRETRY de 3 ms, tPD de 0,7 µs, pulso mínimo de 800 ns;
  as curvas de exemplo usam uma carga de 2 Ω e 1 mH.
