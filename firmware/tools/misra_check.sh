#!/bin/bash
# Confere os nossos modulos (Modules/) contra o MISRA C:2012 com o cppcheck.
# O codigo gerado pelo CubeMX, a HAL e o FreeRTOS ficam de fora.
#
# Uso, a partir da pasta do projeto:   tools/misra_check.sh
# Precisa de:                          sudo apt install cppcheck
cd "$(dirname "$0")/.." || exit 1

cppcheck --addon=misra --platform=arm32-wchar_t4 --std=c11 --quiet --enable=style \
  -DUSE_HAL_DRIVER -DSTM32F103xB -D__GNUC__ \
  -IModules/app -IModules/control -IModules/sensor -IModules/motor -IModules/serial -IModules/debug \
  -ICore/Inc -IDrivers/STM32F1xx_HAL_Driver/Inc \
  -IDrivers/CMSIS/Device/ST/STM32F1xx/Include -IDrivers/CMSIS/Include \
  -IMiddlewares/Third_Party/FreeRTOS/Source/include \
  -IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 \
  --suppress='*:Drivers/*' --suppress='*:Middlewares/*' --suppress='*:Core/*' \
  --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=checkersReport \
  --template='{file}:{line}: {id}: {message}' "$@" Modules
