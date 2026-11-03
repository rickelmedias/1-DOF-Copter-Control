# Verificação das citações de `citacoes.txt` contra a biblioteca

**Data:** 2026-11-02
**Ferramentas:** servidor MCP "biblioteca" (`list_books`, `search_library`, `get_passage`, `get_page_image`).

**Convenções**
- "p." = página **impressa**; "PDF p." = página do arquivo, usada quando a impressa não existe (slides, preprints).
- **OCR**: os trechos de Burns e Wellings (2009), Colton (2007) e Åström e Rundqwist (1989) vêm de OCR. A redação foi conferida pelo sentido, mas confira no PDF antes de fazer uma citação direta.
- Obras **ausentes da biblioteca**: Barry (2016), *Mastering the FreeRTOS Real Time Kernel*; documentação web do FreeRTOS (`freertosmanual`, `freertoscortex`); datasheet STM32F103 (DS5319); DRV8871 (SLVSCY9B); Register Map do MPU-6000/6050; artigo da maxon; NXP AN3461. Busquei termos específicos de cada uma (por exemplo `configMAX_SYSCALL_INTERRUPT_PRIORITY`, `DLPF_CFG`, "800 ns", "atan2 tilt") e só voltaram obras sem relação.
- **Buttazzo**: a biblioteca tem apenas a capa, a ficha catalográfica e o prefácio da **4ª ed.** (2024). Nenhuma afirmação atribuída a `buttazzo2011` pode ser conferida.

---

## [1] introducao.tex — `burns2009` [p.~3]
**Veredito: CONFIRMADO** (OCR)
- Burns e Wellings, Seção 1.1 "Definition of a real-time system", p. 3:
  > "For example, the flight control system of a combat aircraft is a hard real-time system because a missed deadline could lead to a catastrophe, whereas a data acquisition system for a process control application is soft, …"
- **Página:** p. 3 (está correta).

## [2] introducao.tex — `mane2019`
**Veredito: CONFIRMADO**
- O artigo é uma bancada didática: "Efficient engagement of students in teaching-learning process by implementing a closed and open-loop system" (Introdução, PDF p. 1) e "Scope of this lab-setup will help learners to study and validate essential components of control systems" (Conclusão, PDF p. 6).
- O PID foi ajustado de forma empírica, com a biblioteca `PID_v1.h` (Seção "B. Closed Loop Control of 1 DOF Helicopter", PDF p. 5):
  > "For this particular system, the values for K_p, K_i and K_d at stability of 90 and 230 set points for angle and equivalent PWM, are observed to have values typically as 0.08, 0.05, and 0.025 respectively."

  Os ganhos P, PD e PI também foram testados por tentativa (PDF p. 5–6).
- **Página:** o PDF não traz numeração impressa (ICAC3, p. 1–6). Se quiser página, use a p. 5 do artigo.

## [3] introducao.tex — `cuero2020`
**Veredito: CONFIRMADO**
- Resumo, p. 3292:
  > "This document presents the design, implementation, and evaluation of a sliding mode controller to stabilize the angular position of a 1-DOF helicopter. … programed on the STM32F411 microcontroller."
- A planta é uma bancada física ("Flight control unit", Fig. 1, p. 3293).
- **Página:** p. 3292.

## [4] introducao.tex — `uyar2024`
**Veredito: PARCIAL**
- O artigo avalia MPC num *copter* de 1 GDL, mas **só em simulação**, sem bancada experimental. Resumo (PDF p. 1):
  > "Simulation results show that the gradient-based MPC performs well in dynamic and steady-state performance compared to PID controllers …"

  Conclusões, p. 102: a aplicação "to actual hardware systems" aparece como trabalho futuro.
- **Correção sugerida:** "Plataformas desse tipo são empregadas como bancadas didáticas e experimentais de helicópteros de um grau de liberdade, nas quais se avaliam desde controladores PID ajustados empiricamente \cite{mane2019} até técnicas de modos deslizantes \cite{cuero2020}; o mesmo modelo serve ainda à avaliação, em simulação, de controle preditivo \cite{uyar2024}."

## [5] referencial_teorico.tex — `buttazzo2011, burns2009`
**Veredito: PARCIAL** (Burns confirma; Buttazzo não pode ser verificado)
- Burns e Wellings, Seção 1.1, p. 2:
  > "Consequently, the correctness of a real-time system depends not only on the logical result of the computation, but also on the time at which the results are produced."

  A mesma ideia reaparece no Cap. 16 "Conclusions", p. 575.
- O prefácio do Buttazzo (4ª ed.) não traz essa definição.
- **Correção sugerida:** manter a frase e trocar a citação por `\cite[p.~2]{burns2009}`. Se quiser manter `buttazzo2011`, confira a página no Cap. 1 da 3ª ed.

## [6] referencial_teorico.tex — `burns2009` [p.~2--3]
**Veredito: CONFIRMADO** (OCR)
- Seção 1.1, p. 2:
  > "Hard real-time systems are those where it is absolutely imperative that responses occur within the specified deadline. Soft real-time systems are those where response times are important but the system will still function correctly if deadlines are occasionally missed."
- Seção 1.1, p. 3:
  > "A deadline that can be missed occasionally, but in which there is no benefit from late delivery, is called firm."
- **Página:** p. 2–3 (está correta).

## [7] referencial_teorico.tex — `burns2009` [p.~3]
**Veredito: CONFIRMADO** (OCR). É o mesmo trecho do item 1.
- **Página:** p. 3 (está correta).

## [8] referencial_teorico.tex — `rico2022`
**Veredito: CONFIRMADO**
- Resumo (PDF p. 1, artigo p. 134870):
  > "a flexible unit, isolated from body vibrations, houses redundant sensors … a hybrid scheduler (first-come-first-serve scheduling and earliest-deadline-first scheduling) with dynamic priority is proposed. The results obtained show the effectiveness of the system using schedulability tests for uni-processor systems. A set of real data supports the performance of the flight controller."
- Os "dados reais de voo" estão na Seção E "Hardware test", p. 134880: "an experiment is carried out with the drone in stationary flight, which records 100 000 samples".
- O RTOS simplificado aparece na p. 134873: "The aim is for the scheduler to be as simple as possible".
- **Página:** use p. 134870 para o resumo como um todo. Detalhes: escalonador, p. 134873–134875; testes de escalonabilidade, p. 134878; isolamento dos sensores, p. 134871.

## [9] referencial_teorico.tex — `barry2016`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (Barry 2016)
- A parte genérica da frase (escalonamento preemptivo por prioridades fixas) tem apoio em Burns e Wellings, Seção 11.3, p. 370–371. Lá a política é chamada de "preemptive priority-based scheduling". A afirmação sobre o FreeRTOS continua dependendo de Barry.

## [10] referencial_teorico.tex — `burns2009, buttazzo2011`
**Veredito: PARCIAL** (Burns sustenta; Buttazzo não pode ser verificado)
- Burns, Seção 11.2.4, p. 369:
  > "An arbitrarily complex concurrent program cannot easily be analysed to predict its worst-case behaviour. Hence it is necessary to impose some restrictions on the structure of real-time concurrent programs."
- A Tabela 11.2, p. 370, define B (bloqueio), C (WCET), P (prioridade) e T (período).
- A análise de tempo de resposta (Seção 11.5, p. 374–375) e o termo de bloqueio (Seções 11.8–11.9, p. 384–387) mostram que o cumprimento dos prazos é verificado por análise desses parâmetros.
- **Correção sugerida:** manter a frase e citar `\cite[p.~369--375]{burns2009}`, removendo `buttazzo2011` até a página ser conferida na 3ª ed.

## [11] referencial_teorico.tex — `freertosmanual, barry2016`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (as duas fontes estão ausentes).

## [12] referencial_teorico.tex — `barry2016`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.** A técnica de *deferred interrupt processing* tem de ser conferida no Cap. 6 de Barry (2016).

## [13] referencial_teorico.tex — `barry2016`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (Barry).
- Como apoio genérico, Burns, Seção 11.8, p. 384–385, descreve a herança de prioridade e mostra que ela não elimina o bloqueio:
  > "With priority inheritance, a task's priority is no longer static; if a task p is suspended waiting for task q to undertake some computation then the priority of q becomes equal to the priority of p … Note that as a consequence of this algorithm, task b will now suffer blocking even though it does not use a shared object."
- **Sugestão:** acrescentar `\cite[p.~384--385]{burns2009}` ao lado de `barry2016`.

## [14] referencial_teorico.tex — `freertoscortex`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (página web do FreeRTOS).

## [15] referencial_teorico.tex — `stm32f103ds`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (DS5319).

## [16] referencial_teorico.tex — `nxpan3461`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (AN3461). Confira a equação no PDF da NXP antes de citá-la.

## [17] referencial_teorico.tex — `nxpan3461`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (AN3461).
- Outras obras da biblioteca sustentam a mesma ideia e podem servir de citação adicional:
  - Beard e McLain, *Small Unmanned Aircraft*, Fig. 8.4, p. 147–149: "Since this scheme assumes unaccelerated flight during maneuvers where acceleration exists, the estimation error can be unacceptably large."
  - Quan, *Introduction to Multicopter Design and Control*, Seção 7.1.1, p. 149: "in the absence of external forces, the accurate pitch angle and roll angle can be measured … The output error induced by vibration is large".

  Nenhuma das duas está no `references.bib`.

## [18] referencial_teorico.tex — `colton2007`
**Veredito: CONFIRMADO** (OCR)
- Slide "Cons:" do giroscópio, PDF p. 9–10:
  > "The dreaded gyroscopic drift. If the gyro does not read perfectly zero when stationary (and it won't), the small rate will keep adding to the angle until it is far away from the actual angle."
- **Página:** o documento não tem páginas numeradas. Se quiser página, use p. 9–10 (PDF).

## [19] referencial_teorico.tex — `colton2007`
**Veredito: CONFIRMADO** (OCR)
- "More on Digital Filters", PDF p. 14:
  > "For time periods shorter than half a second, the gyroscope integration takes precedence and the noisy horizontal accelerations are filtered out. For time periods longer than half a second, the accelerometer average is given more weighting than the gyroscope, which may have drifted by this point."
- **Página:** PDF p. 14. Os slides do filtro estão nas PDF p. 10–11.

## [20] referencial_teorico.tex — `colton2007`
**Veredito: CONFIRMADO** (OCR). É o mesmo trecho do item 19; a definição de τ = a·dt/(1 − a) está na PDF p. 13–14.
- No exemplo do autor, τ ≈ 0,5 s ("half a second"). A generalização para um τ_f qualquer decorre da definição de constante de tempo da PDF p. 13: "signals much longer than the time constant pass through unaltered while signals shorter than the time constant are filtered out."
- **Página:** PDF p. 13–14.

## [21] referencial_teorico.tex — `astrom1995`
**Veredito: CONFIRMADO**
- Seção 3.3 "PID Control", p. 64, logo após a Eq. (3.9):
  > "The control variable is thus a sum of three terms: the P-term (which is proportional to the error), the I-term (which is proportional to the integral of the error), and the D-term (which is proportional to the derivative of the error)."
- **Página:** p. 64.

## [22] referencial_teorico.tex — `astrom1995` [p.~74]
**Veredito: CONFIRMADO**
- p. 73, Eq. (3.19)–(3.21): o erro da parte derivativa é "e_d = c·y_sp − y". Com c = 0, a derivada passa a atuar apenas sobre a medida.
- p. 74:
  > "The parameter c is normally chosen equal to zero to avoid large transients in the control signal due to sudden changes in the setpoint."
- **Página:** p. 73–74. A definição de c está na p. 73; a justificativa, na p. 74.

## [23] referencial_teorico.tex — `astrom1989`
**Veredito: CONFIRMADO** (OCR)
- Seção 2, p. 1693: "Let us first illustrate the phenomenon of windup. … The output remains saturated at this point because of the large value of the integral term." O termo "integrator windup" está no título e na Fig. 1.
- **Página:** p. 1693.

## [24] referencial_teorico.tex — `astrom1989`
**Veredito: CONFIRMADO** (OCR)
- Seção 2, "Summary", p. 1695:
  > "We have thus given two ad hoc methods for avoiding windup, tracking or back-calculation and conditional integration. These schemes can also be combined."
- Seção 3 "General Methods", "An observer approach", p. 1695. Conclusões, p. 1698: "The observer approach is a good general technique."
- **Página:** p. 1695.

## [25] referencial_teorico.tex — `astrom1995` [p.~83--84]
**Veredito: CONFIRMADO**
- Seção 3.5, "Back-Calculation and Tracking", p. 83–84:
  > "Back-calculation works as follows: When the output saturates, the integral is recomputed so that its new value gives an output at the saturation limit. It is advantageous not to reset the integrator instantaneously but dynamically with a time constant T_t."

  O nome "tracking time constant" aparece na p. 85.
- **Página:** p. 83–84 (está correta).

## [26] referencial_teorico.tex — `astrom2008` [p.~306--308]
**Veredito: CONFIRMADO**
- Seção 10.4 "Integrator Windup", p. 307:
  > "the system has an extra feedback path that is generated by measuring the actual actuator output, or the output of a mathematical model of the saturating actuator, and forming an error signal e_s as the difference between the output of the controller v and the actuator output u. The signal e_s is fed to the input of the integrator through gain k_t."
- **Página:** p. 306–308 está adequado; o trecho exato está na p. 307. O arquivo é a versão eletrônica v2.11b, e a paginação pode diferir da impressão de 2008.

## [27] referencial_teorico.tex — `astrom1995` [p.~88--89]
**Veredito: CONFIRMADO**
- Seção 3.5, "Conditional Integration", p. 88:
  > "In this method integration is switched off when the control is far from steady state. … The method is also called integrator clamping."
- A variante usada no trabalho também está no livro (p. 89):
  > "Integration is switched off when the controller is saturated and the integrator update is such that it causes the control signal to become more saturated."
- **Página:** p. 88–89 (está correta). Se quiser, cite a p. 89 também na segunda oração.

## [28] metodologias.tex — `uyar2024` [p.~98]
**Veredito: PARCIAL (só a página).** A equação foi conferida na imagem da página.
- p. 98, Eq. (2): "J θ̈ + b θ̇ = F_h l_h − m_h g l_h", "derived by the Newton-Euler method".
- A omissão do cos θ só aparece na **p. 99**:
  > "Given that the impact of gravity will fluctuate in accordance with the angle of the rod in relation to the ground, and given that it would introduce a trigonometric element to the equation, rendering the equation non-linear, this effect has not been incorporated into the system model."
- **Correção:** trocar `[p.~98]` por `[p.~98--99]`. O texto pode continuar igual.

## [29] metodologias.tex — `cuero2020`
**Veredito: CONFIRMADO**
- Seção II "System Identification", p. 3293:
  > "However, this requires knowing parameters such as the moment of inertia, motor constants, among others that are not easy to measure. Therefore, a black-box model is a better choice to estimate the transfer function through experimentation, which means to measure the angular displacement when a thrust force is applied."
- Eq. (1) e (2), p. 3293–3294: G(s) de 2ª ordem, entre empuxo (N) e ângulo, identificada com o System Identification Toolbox.
- **Página:** p. 3293.

## [30] metodologias.tex — `stm32f103ds`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.**

## [31] metodologias.tex — `mpu6050rm`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.**

## [32] metodologias.tex — `drv8871ds`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.**

## [33] metodologias.tex — `drv8871ds`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.**

## [34] metodologias.tex — `buttazzo2011, burns2009`
**Veredito: PARCIAL** (Burns sustenta; Buttazzo não pode ser verificado)
- Burns, Tabela 11.2 "Standard notation", p. 370: B "Worst-case blocking time", C "Worst-case execution time (WCET)", P "Priority assigned to the task", T "Minimum time between task releases (task period)".
- O modelo simples (Seção 11.2.4, p. 369–370) exige períodos e WCET conhecidos. O bloqueio entra na análise nas Seções 11.8–11.9, p. 384–387.
- **Correção sugerida:** `\cite[p.~369--370]{burns2009}`, sem `buttazzo2011`, até conferir a 3ª ed.

## [35] metodologias.tex — `barry2016`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.**

## [36] metodologias.tex — `mpu6050rm`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.** O atraso de cerca de 5 ms do DLPF a 44 Hz tem de ser conferido na tabela DLPF_CFG do Register Map.

## [37] metodologias.tex — `buttazzo2011`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (Buttazzo). Burns e Wellings sustentam a mesma ideia e podem substituir a chave.
- Burns, Seção 11.13 "Worst-case execution time", p. 407 (OCR):
  > "Worst-case execution time estimation … can be obtained by either measurement or analysis. The problem with measurement is that it is difficult to be sure when the worst case has been observed."
- **Correção sugerida:** "Ressalta-se que o maior valor observado constitui evidência experimental, mas não prova o pior caso de todas as execuções possíveis: com medições, é difícil assegurar que o pior caso tenha de fato sido observado \cite[p.~407]{burns2009}." Burns não usa literalmente a expressão "limite inferior". Se ela for mantida, a citação deve ficar com o Buttazzo, conferido na 3ª ed.

## [38] metodologias.tex — `maxonchoke`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.**

## [39] metodologias.tex — `drv8871ds`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA.**

## [40] metodologias.tex — `nxpan3461`
**Veredito: NÃO ENCONTRADO NA BIBLIOTECA** (AN3461). Para apoio adicional, veja o item 17 (Beard e McLain, p. 147–149; Quan, p. 149).

## [41] metodologias.tex — `colton2007`
**Veredito: PARCIAL** (OCR)
- Colton não *recomenda* explicitamente usar o acelerômetro "apenas" no longo prazo. A conclusão decorre do projeto do filtro:
  - PDF p. 8, prós do filtro passa-baixas: "Filters out short-duration horizontal accelerations. Only long-term acceleration (gravity) passes through."
  - PDF p. 14: "For time periods longer than half a second, the accelerometer average is given more weighting than the gyroscope".
- **Correção sugerida:** "… necessária ao cálculo da inclinação \cite{nxpan3461}; pela mesma razão, no filtro descrito por \citeonline{colton2007}, o acelerômetro só prevalece em escalas de tempo longas, uma vez que apenas a aceleração de longo prazo, a gravidade, atravessa o filtro passa-baixas."

---

## Resumo

| # | Chave | Veredito | Página sugerida / correção |
|---|---|---|---|
| 1 | burns2009 | CONFIRMADO | p. 3 |
| 2 | mane2019 | CONFIRMADO | PDF p. 5 (sem página impressa) |
| 3 | cuero2020 | CONFIRMADO | p. 3292 |
| 4 | uyar2024 | PARCIAL | Avaliação só em simulação: "…controle preditivo, avaliado em simulação \cite{uyar2024}" |
| 5 | buttazzo2011, burns2009 | PARCIAL | Usar `\cite[p.~2]{burns2009}`; Buttazzo não verificável |
| 6 | burns2009 | CONFIRMADO | p. 2–3 |
| 7 | burns2009 | CONFIRMADO | p. 3 |
| 8 | rico2022 | CONFIRMADO | p. 134870 (resumo); voo estacionário, p. 134880 |
| 9 | barry2016 | NÃO ENCONTRADO | (parte genérica: Burns, p. 370–371) |
| 10 | burns2009, buttazzo2011 | PARCIAL | `\cite[p.~369--375]{burns2009}`; Buttazzo não verificável |
| 11 | freertosmanual, barry2016 | NÃO ENCONTRADO | — |
| 12 | barry2016 | NÃO ENCONTRADO | — |
| 13 | barry2016 | NÃO ENCONTRADO | Acrescentar Burns, p. 384–385 (herança de prioridade) |
| 14 | freertoscortex | NÃO ENCONTRADO | — |
| 15 | stm32f103ds | NÃO ENCONTRADO | — |
| 16 | nxpan3461 | NÃO ENCONTRADO | — |
| 17 | nxpan3461 | NÃO ENCONTRADO | Apoio alternativo: Beard e McLain, p. 147–149 |
| 18 | colton2007 | CONFIRMADO | PDF p. 9–10 |
| 19 | colton2007 | CONFIRMADO | PDF p. 14 |
| 20 | colton2007 | CONFIRMADO | PDF p. 13–14 |
| 21 | astrom1995 | CONFIRMADO | p. 64 |
| 22 | astrom1995 | CONFIRMADO | p. 73–74 (hoje p. 74) |
| 23 | astrom1989 | CONFIRMADO | p. 1693 |
| 24 | astrom1989 | CONFIRMADO | p. 1695 |
| 25 | astrom1995 | CONFIRMADO | p. 83–84 |
| 26 | astrom2008 | CONFIRMADO | p. 307 (306–308 ok) |
| 27 | astrom1995 | CONFIRMADO | p. 88–89 |
| 28 | uyar2024 | PARCIAL | Trocar para p. 98–99 (omissão do cos θ na p. 99) |
| 29 | cuero2020 | CONFIRMADO | p. 3293 |
| 30 | stm32f103ds | NÃO ENCONTRADO | — |
| 31 | mpu6050rm | NÃO ENCONTRADO | — |
| 32 | drv8871ds | NÃO ENCONTRADO | — |
| 33 | drv8871ds | NÃO ENCONTRADO | — |
| 34 | buttazzo2011, burns2009 | PARCIAL | `\cite[p.~369--370]{burns2009}`; Buttazzo não verificável |
| 35 | barry2016 | NÃO ENCONTRADO | — |
| 36 | mpu6050rm | NÃO ENCONTRADO | — |
| 37 | buttazzo2011 | NÃO ENCONTRADO | Trocar por `\cite[p.~407]{burns2009}`, com nova redação ("difícil assegurar que o pior caso tenha sido observado") |
| 38 | maxonchoke | NÃO ENCONTRADO | — |
| 39 | drv8871ds | NÃO ENCONTRADO | — |
| 40 | nxpan3461 | NÃO ENCONTRADO | Apoio alternativo: Beard e McLain, p. 147–149 |
| 41 | colton2007 | PARCIAL | Colton não "recomenda"; reescrever como "o acelerômetro só prevalece em escalas longas" (PDF p. 8, 14) |

**Contagem:** 17 CONFIRMADO · 6 PARCIAL · 18 NÃO ENCONTRADO · 0 CONTRADIZ.
