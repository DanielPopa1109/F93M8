# Project-local source rules; regenerate from CubeIDE after configuration changes.
C_SRCS += ../BSW/Crc/Crc.c

C_DEPS += ./BSW/Crc/Crc.d

OBJS += ./BSW/Crc/Crc.o

BSW/Crc/%.o BSW/Crc/%.su BSW/Crc/%.cyclo: ../BSW/Crc/%.c BSW/Crc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I"../BSW/Com/PduR" -I"../BSW/Com/CanTp" -I"../BSW/Com/CanIf" -I"../BSW/Com/Can" -I"../BSW/Crc" -I"../BSW/Com" -I"../BSW" -I"../Core/Inc" -I../Core/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
clean: clean-BSW-2f-Crc
clean-BSW-2f-Crc:
	-$(RM) ./BSW/Crc/Crc.o ./BSW/Crc/Crc.d ./BSW/Crc/Crc.su ./BSW/Crc/Crc.cyclo
.PHONY: clean-BSW-2f-Crc
