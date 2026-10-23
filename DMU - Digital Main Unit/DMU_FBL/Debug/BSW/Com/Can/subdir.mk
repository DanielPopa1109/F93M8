# Project-local source rules; regenerate from CubeIDE after configuration changes.
C_SRCS += ../BSW/Com/Can/Can.c

C_DEPS += ./BSW/Com/Can/Can.d

OBJS += ./BSW/Com/Can/Can.o

BSW/Com/Can/%.o BSW/Com/Can/%.su BSW/Com/Can/%.cyclo: ../BSW/Com/Can/%.c BSW/Com/Can/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I"../BSW/Com/PduR" -I"../BSW/Com/CanTp" -I"../BSW/Com/CanIf" -I"../BSW/Com/Can" -I"../BSW/Crc" -I"../BSW/Com" -I"../BSW" -I"../Core/Inc" -I../Core/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
clean: clean-BSW-2f-Com-2f-Can
clean-BSW-2f-Com-2f-Can:
	-$(RM) ./BSW/Com/Can/Can.o ./BSW/Com/Can/Can.d ./BSW/Com/Can/Can.su ./BSW/Com/Can/Can.cyclo
.PHONY: clean-BSW-2f-Com-2f-Can
