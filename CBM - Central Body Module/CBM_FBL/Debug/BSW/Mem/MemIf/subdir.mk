# Project-local source rules; regenerate from CubeIDE after configuration changes.
C_SRCS += ../BSW/Mem/MemIf/MemIf.c

C_DEPS += ./BSW/Mem/MemIf/MemIf.d

OBJS += ./BSW/Mem/MemIf/MemIf.o

BSW/Mem/MemIf/%.o BSW/Mem/MemIf/%.su BSW/Mem/MemIf/%.cyclo: ../BSW/Mem/MemIf/%.c BSW/Mem/MemIf/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I"../BSW/Com/PduR" -I"../BSW/Com/CanTp" -I"../BSW/Com/CanIf" -I"../BSW/Com/Can" -I"../BSW/Mem/MemIf" -I"../BSW/Mem/Fls" -I"../BSW/Mem/Crc" -I"../BSW/Mem" -I"../BSW/Crc" -I"../BSW/Com" -I"../BSW" -I"../Core/Inc" -I../Core/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
clean: clean-BSW-2f-Mem-2f-MemIf
clean-BSW-2f-Mem-2f-MemIf:
	-$(RM) ./BSW/Mem/MemIf/MemIf.o ./BSW/Mem/MemIf/MemIf.d ./BSW/Mem/MemIf/MemIf.su ./BSW/Mem/MemIf/MemIf.cyclo
.PHONY: clean-BSW-2f-Mem-2f-MemIf
