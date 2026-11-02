# Binário final do firmware

Versão gravada na placa ao fim do trabalho (02/11/2026): Kp 0,06, Ki 0,08, Kd 0,04, anti-windup ligado, corte por ângulo (+60° / −90°), correção lenta do ângulo em voo e telemetria de tempo. Gerada com STM32CubeIDE 2.2.0 (GNU Tools for STM32 14.3.rel1), perfil Debug.

| Arquivo | Uso |
|---|---|
| `topicos2_final.hex` | Gravação pelo STM32CubeProgrammer (traz os endereços) |
| `topicos2_final.bin` | Gravação em 0x08000000 |
| `SHA256SUMS.txt` | Conferência dos arquivos |

## Regravar sem compilar

Com a placa conectada por ST-Link (SWD):

```sh
STM32_Programmer_CLI -c port=SWD freq=100 mode=HOTPLUG -e all -w topicos2_final.hex -v -rst
```

Espere aparecer `Download verified successfully`; se aparecer falha de apagamento, tente de novo. Com a interface gráfica do STM32CubeProgrammer: *Open file* e escolha o `.hex`, depois *Download*. A frequência de 100 kHz em *hot plug* foi a que conectou de forma confiável nesta bancada.

**Segurança:** antes de gravar, desligue o motor (ou os 12 V) e deixe a haste em repouso. O firmware arma o motor sozinho cerca de 2 s depois de ligar, com a calibração feita com a haste parada.

## Conferir

```sh
sha256sum -c SHA256SUMS.txt
```
