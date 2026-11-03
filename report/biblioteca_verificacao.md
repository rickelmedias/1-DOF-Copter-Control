# Verificação de referências com a biblioteca (MCP "biblioteca")

**Data:** 2026-11-02
**Ferramentas usadas:** `list_books`, `search_library`, `get_passage`, `get_page_image` do servidor MCP "biblioteca".

Convenções: "p." é a página **impressa**; "PDF p." é a página do arquivo quando a impressa não veio nos metadados. Trechos com origem **OCR** podem ter erros de reconhecimento e devem ser conferidos no PDF antes de uma citação direta. As equações dos itens 7 e 9 foram conferidas na imagem da página.

---

## 1. Buttazzo, *Hard Real-Time Computing Systems*

**Resultado: o exemplar na biblioteca NÃO é a 3ª ed. (2011) e só contém o prefácio.**

- O arquivo tem 19 trechos: capa, ficha catalográfica, prefácio e agradecimentos (PDF p. 3–10). Não há capítulos.
- Ficha catalográfica (PDF p. 4): "ISBN 978-3-031-45409-7 … 3rd edition: © Springer Science+Business Media, LLC 2011 / 4th edition: © … Springer Nature Switzerland AG 2024". Capa: "Fourth Edition / Springer". Prefácio: seção "Difference with the Third Edition".
- **Lista de aplicações** (Preface, p. v, PDF p. 5):
  > "Examples of applications that require real-time computing include nuclear power plants, railway switching systems, automotive and avionic systems, air traffic control, telecommunications, robotics, and military systems. In the last years, real-time computing was also demanded in new applications areas, such as medical equipment, consumer electronics, multimedia systems, flight simulation systems, virtual reality, and interactive games."

  → **Não aparece "flight control systems"**; aparecem "avionic systems", "air traffic control" e "flight simulation systems" (4ª ed., p. v).
- **Definições de hard/firm/soft:** não encontradas (estão no Cap. 1, que não está no arquivo).
- **Tempos de execução medidos não garantem o WCET:** não encontrado.

## 2. Burns & Wellings, *Real-Time Systems and Programming Languages*, 4ª ed.

**Resultado: confirmado** (origem OCR; os metadados do arquivo dizem 1988, mas a ficha impressa diz 4ª ed., © 1989, 2009).

- Folha de rosto (PDF p. 4–5): "Ada, Real-Time Java and C/Real-Time POSIX / Fourth Edition / Alan Burns and Andy Wellings / University of York / Pearson Education Limited Edinburgh Gate Harlow Essex CM20 2JE England".
- Ficha: "© Pearson Education Limited 1989, 2009 … ISBN: 978-0-321-41745-9".
  → **Editora:** Pearson Education Limited (selo Addison-Wesley). **Cidade:** Harlow (Essex, Inglaterra).
- **Hard/soft**, Seção 1.1 "Definition of a real-time system", p. 2:
  > "Hard real-time systems are those where it is absolutely imperative that responses occur within the specified deadline. Soft real-time systems are those where response times are important but the system will still function correctly if deadlines are occasionally missed."
- **Exemplo de controle de voo**, Seção 1.1, p. 3:
  > "For example, the flight control system of a combat aircraft is a hard real-time system because a missed deadline could lead to a catastrophe, whereas a data acquisition system for a process control application is soft, …"
- **Firm**, Seção 1.1, p. 3:
  > "A deadline that can be missed occasionally, but in which there is no benefit from late delivery, is called firm."
- **Firm (outra formulação)**, Seção 9.6 "Temporal scopes", p. 329:
  > "A non-hard task that has a fixed rigid deadline (that is, a tardy service is useless/valueless) is said to be firm."

## 3. Åström & Hägglund, *PID Controllers: Theory, Design, and Tuning*, 2ª ed. (1995)

**Resultado: confirmado.**

- Ficha (PDF p. 2): "Copyright © 1995 by Instrument Society of America, 67 Alexander Drive, P.O. Box 12277, Research Triangle Park, NC 27709 … ISBN 1-55617-516-7 … 2nd ed".
- **Ponderação do setpoint e derivada na saída**, Seção 3.4 "Modifications of the PID Algorithm", "Setpoint Weighting", p. 74:
  > "The parameter c is normally chosen equal to zero to avoid large transients in the control signal due to sudden changes in the setpoint."

  > "The controller with b = 0 and c = 0 is sometimes called an I-PD controller, and the controller with b = 1 and c = 0 is sometimes called a PI-D controller."

  (c = 0 significa que a derivada age apenas sobre a saída do processo.)
- **Anti-windup**, Seção 3.5 "Integrator Windup":
  - *Back-calculation and Tracking*, p. 83–84:
    > "Back-calculation works as follows: When the output saturates, the integral is recomputed so that its new value gives an output at the saturation limit. It is advantageous not to reset the integrator instantaneously but dynamically with a time constant T_t."
  - *Conditional Integration* (= clamping), p. 88–89:
    > "Conditional integration is an alternative to back-calculation or tracking. In this method integration is switched off when the control is far from steady state. … The method is also called integrator clamping."
  - A mesma seção também trata de *Incremental Algorithms* (p. 82), *Controllers with a Tracking Mode* (p. 86–87) e *Combined Schemes* (p. 92).

## 4. Åström & Rundqwist, "Integrator Windup and How to Avoid It" (ACC 1989)

**Resultado: confirmado** (origem OCR; os metadados do arquivo citam "Bemporad" como autor, mas o artigo impresso traz "Karl Johan Åström and Lars Rundqwist, Lund Institute of Technology"; p. 1693–1698).

- Métodos descritos: Seção 2, "Ad hoc methods": **back-calculation/tracking** e **conditional integration** (além de esquemas combinados, cutback e preloading); Seção 3, "General Methods": **abordagem por observador** (baseada em espaço de estados).
- Resumo da Seção 2, p. 1695:
  > "We have thus given two ad hoc methods for avoiding windup, tracking or back-calculation and conditional integration. These schemes can also be combined."
- Conclusões, p. 1698:
  > "Conditional integration is easy to apply to most controllers. … Tracking is another good method for avoiding windup. … The observer approach is a good general technique."
- O termo "clamping" aparece apenas no esquema de cutback ("The integrator is clamped when the predicted process output is outside the cutback interval", p. 1695).

## 5. Åström & Murray, *Feedback Systems* (2008)

**Resultado: confirmado.** O arquivo é a edição eletrônica v2.11b (28 set. 2012); a paginação pode diferir da impressa.

- Ficha: "Copyright © 2008 by Princeton University Press … Princeton, New Jersey 08540 … ISBN-13: 978-0-691-13576-2 … ISBN-10: 0-691-13576-2".
- **Anti-windup:** Seção 10.4 "Integrator Windup", p. 306–308 (Exemplos 10.5 e 10.6, Fig. 10.10 e 10.11). Trecho da p. 307:
  > "There are many methods to avoid windup. One method is illustrated in Figure 10.11: the system has an extra feedback path that is generated by measuring the actual actuator output, or the output of a mathematical model of the saturating actuator, and forming an error signal e_s as the difference between the output of the controller v and the actuator output u. The signal e_s is fed to the input of the integrator through gain k_t."
- Integração condicional aparece no Exercício 10.11, p. 314.

## 6. Mane, Mahapatro, Gundecha (2019), "Single Degree of Freedom Helicopter Model: Laboratory Setup Design"

**Resultado: NÃO há equação de movimento no artigo.**

- Autores (PDF p. 1): "Rohit Mane, Kaliprasad A. Mahapatro, Aniket D. Gundecha — MIT Academy of Engineering, Pune / Avantika University, Ujjain".
- A seção "B. Mechanical Modelling" (PDF p. 2–4) descreve só a construção mecânica (bloco, placas, haste de alumínio, potenciômetro de 100 kΩ). Não há equação dinâmica. O controle é um PID empírico (biblioteca `PID_v1.h`, Arduino Uno). Trecho (PDF p. 5):
  > "For this particular system, the values for K_p, K_i and K_d at stability of 90 and 230 set points for angle and equivalent PWM, are observed to have values typically as 0.08, 0.05, and 0.025 respectively."
- Observação: o texto se contradiz quanto a K_i (0.05 aqui; "K_i = 0.005" logo depois).
- Sensor: potenciômetro de 100 kΩ, leitura de 0–1023 mapeada para 0–180° (PDF p. 1–2 e p. 4).
- → Se o relatório atribui a este artigo uma equação J·θ̈ = …, essa atribuição está errada.

## 7. Cuero, Vargas, Jacinto (2020), IJERT

**Resultado: NÃO há equação de movimento; o modelo é uma função de transferência identificada (caixa-preta). Sensor/filtro não informados.**

- Cabeçalho (imagem da p. 3292): "International Journal of Engineering Research and Technology. ISSN 0974-3154, Volume 13, Number 11 (2020), pp. 3292-3297, © International Research Publication House, doi 10.37624/IJERT/13.11.2020.3292-3297".
  → **Atenção:** este é o IJERT da *International Research Publication House* (ISSN 0974-3154), não o ijert.org (ISSN 2278-0181).
- Título: "Flight Control of a 1-DOF Helicopter System using a Sliding Mode Controller for Disturbance Rejection".
- Seção II "System Identification", p. 3293:
  > "However, this requires knowing parameters such as the moment of inertia, motor constants, among others that are not easy to measure. Therefore, a black-box model is a better choice to estimate the transfer function through experimentation, …"
- Eq. (1), p. 3293: G(s) = Y(s)/U(s) = K / (T_ω² s² + 2ζ T_ω s + 1)
- Eq. (2), p. 3293–3294: G(s) = 66.53 / (0.07641 s² + 0.2721 s + 1)   (entrada: força de empuxo em N; saída: ângulo)
- Outros dados: amostragem de 10 ms (p. 3294); implementação em STM32F411.
- **Sensor/filtro:** nenhuma menção a sensor de ângulo, IMU ou filtro nos trechos indexados.

## 8. Colton, "The Balance Filter"

**Resultado: confirmado** (origem OCR; slides sem número de página impressa).

- Capa (PDF p. 1): "A Simple Solution for Integrating Accelerometer and Gyroscope Measurements for a Balancing Platform / Shane Colton <scolton@mit.edu> Mentor, FRC 97 / Rev.1: Submitted as a Chief Delphi white paper - June 25, 2007."
- **Equação do filtro complementar** (PDF p. 10–11):
  > "angle = (0.98)*(angle + gyro * dt) + (0.02)*(x_acc);"
- **Coeficiente** (PDF p. 13–14):
  > "τ = a·dt/(1 − a) ⇔ a = τ/(τ + dt)"

  Exemplo: a = 0.98, dt = 0.01 s → τ = 0.49 s.
- **Acelerômetro** (PDF p. 2): "Measures 'acceleration,' but really force per unit mass." Nos slides de prós e contras, o ruído de "horizontal acceleration" é apontado como a limitação do acelerômetro (PDF p. 14–15: "the lower the time constant, the more horizontal acceleration noise will be allowed to pass through").
- **Deriva do giroscópio** (PDF p. 9–10):
  > "The dreaded gyroscopic drift. If the gyro does not read perfectly zero when stationary (and it won't), the small rate will keep adding to the angle until it is far away from the actual angle."

## 9. Uyar (2024), IJAMEC

**Resultado: confirmado; equação conferida na imagem da página.**

- Cabeçalho (imagem da PDF p. 1): "International Journal of Applied Methods in Electronics and Computers 12(4): 097-103, 2024 … Volume 12, Issue 04, December, 2024 … DOI: 10.58190/ijamec.2024.110". Autor: Okan Uyar, Selçuk University. Recebido em 7 nov. 2024; aceito em 9 dez. 2024.
  → **Vol. 12, n. 4, p. 97–103.**
- Seção 2.1 "One DOF Copter Model", p. 98:
  - Eq. (1): J = m_h l_h²
  - Eq. (2): "The dynamic equation is expressed in the following form, derived by the Newton-Euler method:"
    J θ̈ + b θ̇ = F_h l_h − m_h g l_h
    (θ̈: aceleração angular; θ̇: velocidade angular; b: coeficiente de atrito viscoso)
- p. 99: o autor afirma que o efeito de cos θ da gravidade não foi incorporado, para manter o modelo linear. Motor: F_h = K_t I_m (3), V_m = R I_m + K_e θ̇ (4). FT (8): θ(s)/V_m(s) = (l_h K_t / R) / (J s² + (b + l_h K_t K_e / R) s).

## 10. MPU-6000/MPU-6050 Register Map and Descriptions, rev. 4.2

**Resultado: NÃO ENCONTRADO na biblioteca.** Nenhum documento da InvenSense está indexado. A busca por "DLPF_CFG … bandwidth delay" só retornou o Colton e livros não relacionados. A linha DLPF_CFG = 3 e a data de publicação precisam ser conferidas no PDF do datasheet.

---

## Resumo

| # | Referência | Situação |
|---|---|---|
| 1 | Buttazzo | Só existe a 4ª ed. (2024), e apenas o prefácio. A lista de aplicações traz "avionic systems" e "flight simulation systems", mas não "flight control systems". Definições hard/firm/soft e a frase sobre WCET: não encontradas. |
| 2 | Burns & Wellings | Confirmado. ISBN 978-0-321-41745-9; Pearson Education Ltd, Harlow. Inclui o exemplo do controle de voo como hard. |
| 3 | Åström & Hägglund | Confirmado. ISBN 1-55617-516-7; setpoint weighting na p. 74; anti-windup nas p. 82–92. |
| 4 | Åström & Rundqwist | Confirmado: tracking/back-calculation, integração condicional e abordagem por observador. |
| 5 | Åström & Murray | Confirmado. Seção 10.4, p. 306–308; ISBN 978-0-691-13576-2. |
| 6 | Mane et al. | Contradiz: o artigo não tem equação de movimento. |
| 7 | Cuero et al. | Contradiz: não há equação de movimento, apenas uma FT identificada (eq. 2). Sensor/filtro não citados. O periódico é o IJERT da IRPH (vol. 13, n. 11, p. 3292–3297). |
| 8 | Colton | Confirmado. Rev. 1, 25 jun. 2007. |
| 9 | Uyar | Confirmado. 12(4):97–103, 2024; J θ̈ + b θ̇ = F_h l_h − m_h g l_h. |
| 10 | MPU-6050 Register Map | Não encontrado na biblioteca. |
