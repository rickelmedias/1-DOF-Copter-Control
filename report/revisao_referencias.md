# Revisão das referências e da escrita

Primeira passada: Crossref, DOAJ e PDFs oficiais. Segunda passada: biblioteca MCP (trechos literais com página em `biblioteca_verificacao.md`). A seção "Correções após a biblioteca" no fim deste arquivo prevalece sobre as anteriores.

## Correções no `references.bib`

| Referência | O que mudou | Fonte da verificação |
|---|---|---|
| FreeRTOS Developer Documentation | URL trocada para `.../02-Kernel-features/00-Developer-docs`; título "FreeRTOS Kernel: Developer Documentation" | URL informada pelo usuário |
| Running the RTOS on a ARM Cortex-M Core | URL exatamente como informada (`freertos.org/...RTOS-Cortex-M3-M4`) | URL informada pelo usuário |
| Buttazzo (2011) | Local corrigido de New York para Boston (Springer US); acrescentados série e ISBN | Crossref, DOI 10.1007/978-1-4614-0676-1 |
| Burns e Wellings (2009) | Acrescentado ISBN | **Pendente** (dados de memória) |
| Åström e Rundqwist (1989) | Era `@article` com o nome do congresso como revista; virou `@inproceedings` (American Control Conference, Pittsburgh, IEEE, p. 1693–1698) | Crossref, DOI 10.23919/ACC.1989.4790464 |
| Åström e Hägglund (1995) | Acrescentado ISBN | **Pendente** |
| Åström e Murray (2008) | Acrescentado ISBN | **Pendente** |
| Mane, Mahapatro e Gundecha (2019) | Era `@inproceedings` com "6ª edição" sem confirmação; removido o número da edição, mantidos ICAC3, Mumbai, IEEE, p. 1–6 | Crossref |
| Cuero, Vargas e Jacinto (2020) | Confirmados vol. 13, n. 11, p. 3292–3297; acrescentados ISSN e local da editora (International Research Publication House) | Crossref |
| Uyar (2024) | Confirmados autor, revista e DOI; acrescentado ISSN. **Vol. 12, n. 4, p. 97–103 não constam no Crossref** | Crossref (parcial) |
| Rico, Rico-Azagra e Gil-Martínez (2022) | Confirmado (IEEE Access, v. 10, p. 134870–134883) | Crossref e DOAJ |
| Colton (2007) | Removido "Chief Delphi"; mantido como white paper do MIT, Rev. 1 | **Pendente** (PDF do MIT não abriu) |
| Pedley, AN3461 | Confirmado (Freescale, Rev. 6, 03/2013) | PDF da NXP |
| DRV8871 | Confirmado SLVSCY9B, revisão B, jul. 2016 | PDF da TI |
| MPU-6000/6050 Register Map | Mantido (rev. 4.2, 2013) | **Pendente** (o link da TDK redireciona para a busca do site) |
| STM32F103, maxon | Mantidos, com local da organização | — |
| Duong et al. (2025), Kalman | **Removida**: o trabalho usa filtro complementar, e a revista não tinha volume e número confirmados | — |

## Correções de conteúdo no texto

1. **`\cite{w,...}` na Introdução**: a chave `w` não existia na bibliografia, então a citação saía quebrada no PDF. Foi trocada por `buttazzo2011`, cujo Capítulo 1 lista sistemas de controle de voo entre as aplicações de tempo real (**pendente** de confirmar a página na biblioteca).
2. **Hard/soft/firm (Buttazzo; Burns e Wellings)**: o texto dizia que, em *hard*, "perder um prazo equivale a uma falha". Agora segue a definição de Buttazzo: *hard*, quando a perda pode causar consequências catastróficas; *firm*, quando o resultado atrasado não tem utilidade mas não causa dano; *soft*, quando o resultado atrasado ainda tem alguma utilidade, com desempenho degradado.
3. **"O critério de divisão por período, prioridade e bloqueio é o recomendado na literatura"**: era uma afirmação que nenhum dos dois livros faz literalmente. Foi reescrita: a organização é *coerente com o modelo de tarefas* da análise por prioridades fixas (período, tempo de execução, prioridade, bloqueio) e com a recomendação de Barry de manter as ISRs curtas.
4. **RTOS "primitivas com tempo limitado" (Barry)**: a afirmação foi removida, porque o livro descreve o escalonamento preemptivo por prioridades fixas e não garante isso. Também ficou explícito que a herança de prioridade *reduz, mas não elimina*, a inversão.
5. **Anti-windup (Åström e Rundqwist; Åström e Hägglund)**: o texto chamava *clamping* e integração condicional de "técnicas clássicas". Agora cita o rastreamento (*back-calculation*) e a integração condicional como os métodos discutidos, e o *clamping* como alternativa mais simples.
6. **Filtro complementar (Colton)**: a equação e α = τ/(τ+T_s) foram mantidas, e as variáveis passaram a ser definidas abaixo da equação.
7. **Tilt Sensing (AN3461)**: **mantida**, porque agora tem uma função clara: justifica a fórmula `atan2(-ax, az)` (eq. 29 da nota) e a hipótese de "sem aceleração linear", que é exatamente o que a vibração da hélice viola. Ela também substitui a referência ao Kalman na Seção de ruído.
8. **Modelo da haste (Mane; Cuero; Uyar)**: o texto dizia que a equação tinha "a mesma forma" dos artigos. Como não foi possível conferir as equações deles sem a biblioteca, a frase passou a dizer "análogo aos modelos adotados". **Pendente**: comparar a Eq. 1 com as equações dos três artigos.
9. **DRV8871**: foram confirmados 6,5–45 V, OCP de 3,7–6,4 A, religação em 3 ms e pulso mínimo de 800 ns. Acrescentei a nota do datasheet de que, a 200 kHz, a razão cíclica útil é de 16% a 84%, o que reforça a explicação da falha a 200 kHz (pairar exige cerca de 20%).
10. **Medição de tempo (Buttazzo)**: o texto agora diz que a medição dá apenas um limite inferior do pior caso.
11. **"0,07°/s, valor típico desse sensor"**: o trecho "valor típico" foi removido, porque não tinha fonte.

## Escrita

O texto foi passado para o registro impessoal usado nos trabalhos de referência (CBA 2026, SBC e as teses do ITA): voz passiva e sujeito indeterminado ("optou-se", "observa-se", "conclui-se"), conectivos formais ("uma vez que", "desse modo", "por sua vez"), introdução de capítulo, definição das variáveis após cada equação e citações narrativas ("Segundo Buttazzo (2011)", "Rico et al. (2022) propõem"). Expressões coloquiais como "não dá para", "daí", "uns", "só" e "empurrão" foram substituídas. Os números, tabelas e figuras não foram alterados.

Arquivos alterados: `references.bib`, `parts/body/{introducao,referencial_teorico,metodologias,resultados,conclusao}.tex`, `parts/front_matter/resumo.tex`. O diff completo está em `alteracoes.diff`.

## Correções após a biblioteca

1. **Hard/firm/soft**: o Buttazzo da biblioteca é a 4ª ed. e tem só o prefácio, então as definições passaram a citar Burns e Wellings (2009, p. 2–3), com o texto conferido. O exemplo "flight control system of a combat aircraft is a hard real-time system" (p. 3) agora sustenta a frase da Introdução, que antes citava Buttazzo. O prefácio do Buttazzo não lista "flight control".
2. **Modelo da haste**: Mane et al. (2019) não têm equação de movimento (o PID deles é empírico), e Cuero et al. (2020) usam uma função de transferência identificada (caixa-preta). Por isso a Eq. 1 deixou de ser atribuída aos três. Agora o texto compara com o modelo de Uyar (2024, p. 98), J θ̈ + b θ̇ = F_h l_h − m_h g l_h, que despreza o cos θ, e cita Cuero como precedente da identificação experimental. Na Introdução, cada artigo passou a ser citado pelo que de fato faz (PID empírico, modos deslizantes, MPC).
3. **Clamping**: em Åström e Hägglund (1995, p. 88–89), "integrator clamping" é outro nome da integração condicional. O texto foi corrigido e cita back-calculation (p. 83–84), Åström e Murray (2008, Seção 10.4, p. 306–308) e Åström e Rundqwist (dois métodos ad hoc mais o observador).
4. **Derivada na medida**: agora cita Åström e Hägglund (1995, p. 74), que tratam do peso c = 0 na ponderação da referência.
5. **Colton**: a referência voltou a ser "Chief Delphi White Paper, Rev. 1, 25 jun. 2007", como impresso na capa. A equação e α = τ/(τ+dt) foram confirmadas. Foi acrescentada a descrição da deriva do giroscópio feita pelo autor.
6. **Burns e Wellings**: a editora passou para Pearson Education (Harlow). O ISBN foi confirmado.
7. **Uyar**: vol. 12, n. 4, p. 97–103 confirmados.
8. **Ainda pendentes**: a frase "medições fornecem apenas um limite inferior do pior caso", atribuída a Buttazzo (2011), e o Register Map do MPU-6050 (linha DLPF_CFG = 3), porque nenhum dos dois está na biblioteca. Para fechar, adicionem à biblioteca a 3ª ed. completa do Buttazzo e o PDF do Register Map.

## Validação frase a frase (41 citações)

As 41 frases com `\cite`/`\citeonline` (lista em `citacoes.txt`) foram buscadas por palavra-chave na biblioteca. Resultado: 17 confirmadas, 6 parcialmente sustentadas, 18 com fonte fora da biblioteca e nenhuma contradita. Os trechos literais estão em `verificacao_citacoes.md`.

Correções aplicadas:
- **Uyar (#4):** o MPC dele foi avaliado só em simulação, e o texto agora diz isso. A página passou para p. 98–99 (#28), porque a omissão do cos θ é explicada na p. 99.
- **Buttazzo (#5, #10, #34, #37):** a biblioteca só tem o prefácio da 4ª ed. Essas frases agora citam Burns e Wellings, com páginas: p. 2; p. 369–375; p. 369–370; p. 407. A #37 foi reescrita conforme Burns (é difícil garantir que o pior caso foi observado) e não fala mais em "limite inferior". O Buttazzo virou a 4ª ed. (2024) e ficou citado só na Introdução, p. v, onde lista os sistemas aviônicos como aplicação de tempo real.
- **Herança de prioridade (#13):** acrescentado Burns p. 384–385 (o bloqueio continua existindo).
- **Colton (#41):** a frase "recomenda usar o acelerômetro apenas no longo prazo" foi trocada por "no filtro complementar o acelerômetro só prevalece em escalas longas" (p. 8 e 14).
- **Aceleração e inclinação (#17, #40):** acrescentados Beard e McLain (2012, p. 147–149) e Quan (2017, p. 149), que estão na biblioteca, como apoio ao AN3461. **Confira os dados bibliográficos dessas duas novas entradas** (escritos de memória: Princeton UP 2012, ISBN 978-0-691-14921-9; Springer 2017, ISBN 978-981-10-3381-0).
- **Páginas acrescentadas às confirmadas:** Burns p. 2–3, Mane p. 5, Cuero p. 3292 e 3293, Rico p. 134870, Colton p. 9–10, 13–14 e 14, Åström e Hägglund p. 64 e 73–74, Åström e Rundqwist p. 1693 e 1695, Åström e Murray p. 307.

Ficaram sem verificação na biblioteca, porque os documentos não estão lá: Barry/FreeRTOS, STM32, AN3461, MPU-6050, DRV8871 e maxon. O DRV8871 e o AN3461 foram conferidos nos PDFs oficiais na primeira passada.

## Conferência das fórmulas e contas

Fórmulas, conferidas contra a fonte e contra o código do firmware:
- **Filtro complementar (Eq. 2):** a forma do Colton foi conferida na biblioteca (imagem da página) e na tradução publicada na web. A fórmula α = τ/(τ+T_s) bate com o exemplo numérico dele: τ = 0,75 s e dt = 0,0262 s dão α = 0,966. Também é igual a `angle_filter.c`: est = θ + ω·dt; est += (1−α)(θ_a − est).
- **Inclinação pelo acelerômetro:** θ = atan(−G_x/G_z) é a Eq. 29 do AN3461 (PDF oficial). O firmware usa `atan2(-ax, az)`.
- **PID (Eq. 3):** igual a `pid.c` (u = Kp·e + I − Kd·ω_f; I += Ki·e·dt; integração condicional com u > máx e e > 0). O texto passou a dizer que a derivada é filtrada (5 ms) e quais são os limites de saída e do integrador.
- **Modelo (Eq. 1) e dinâmica do empuxo:** iguais a `sim_1dof.py`, em forma normalizada: G = 300 °/s² por 1% × 20,2% = 6060. A linearização em θ = 0 (d cos θ/dθ = 0) está correta.

Contas refeitas:
- TIM2 = 72 MHz/7200/50 = 200 Hz; TIM1 = 72 MHz/720 = 100 kHz; 13,9 ns por ciclo; 34 870 × 5 ms = 174 s; 153 773 × 5 ms = 769 s = 12,8 min; 580/5000 = 11,6%; 13,5°/190 s = 0,071 °/s = 4,3 °/min; uso de pilha entre 25% e 34%; IAE 94,6/132,0 → −28%; 101,5/94,6 → 7%; T4 e T5 de 16% a 17%. Todas conferem.
- Margens recalculadas com `arm_margins.py`: 3,6° e 18,4 rad/s para 0,30/0,15/0,09; 42,7° nominal para os ganhos de projeto. Todas conferem.

Erros corrigidos:
1. "pelo menos 33°" de margem no pior caso → **32,5°**, que é o valor calculado e o que aparece no Capítulo de refino.
2. T2 com "9% a mais" de erro → **10%** (145,4/132,0 = 1,10).
3. Anti-windup "multiplica por dez" o tempo de recuperação → **cerca de sete** (7,28/1,01 = 7,2).
4. 20 kHz e 1 kHz "acima de 4 A" → **da ordem de 4 A em regime (3,96 e 4,02 A) e até 4,7 A na partida**, segundo `pwm_heat.py`.

Ainda não verificado: a linha DLPF_CFG = 3 do Register Map do MPU-6050 (44 Hz e 4,9 ms). O PDF oficial não abriu e o documento não está na biblioteca.
