# Project-local source rules; regenerate from CubeIDE after configuration changes.
C_SRCS += ../BSW/Mem/Fls/Fls.c

C_DEPS += ./BSW/Mem/Fls/Fls.d

OBJS += ./BSW/Mem/Fls/Fls.o

BSW/Mem/Fls/%.o BSW/Mem/Fls/%.su BSW/Mem/Fls/%.cyclo: ../BSW/Mem/Fls/%.c BSW/Mem/Fls/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I"../BSW/Com/PduR" -I"../BSW/Com/CanTp" -I"../BSW/Com/CanIf" -I"../BSW/Com/Can" -I"../BSW/Mem/MemIf" -I"../BSW/Mem/Fls" -I"../BSW/Mem/Crc" -I"../BSW/Mem" -I"../BSW/Crc" -I"../BSW/Com" -I"../BSW" -I"../Core/Inc" -I../Core/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
clean: clean-BSW-2f-Mem-2f-Fls
clean-BSW-2f-Mem-2f-Fls:
	-$(RM) ./BSW/Mem/Fls/Fls.o ./BSW/Mem/Fls/Fls.d ./BSW/Mem/Fls/Fls.su ./BSW/Mem/Fls/Fls.cyclo
.PHONY: clean-BSW-2f-Mem-2f-Fls
