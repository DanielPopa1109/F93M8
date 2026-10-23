# Project-local source rules; regenerate from CubeIDE after configuration changes.
C_SRCS += ../BSW/Com/CanIf/CanIf.c

C_DEPS += ./BSW/Com/CanIf/CanIf.d

OBJS += ./BSW/Com/CanIf/CanIf.o

BSW/Com/CanIf/%.o BSW/Com/CanIf/%.su BSW/Com/CanIf/%.cyclo: ../BSW/Com/CanIf/%.c BSW/Com/CanIf/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I"../APPL" -I"../BSW/Com/ComM" -I"../BSW/Com/Com" -I"../BSW/Com/CanSM" -I"../BSW/Com/CanNm" -I"../BSW/Mem/MemIf" -I"../BSW/Mem/Fls" -I"../BSW/Mem/Crc" -I"../BSW/Sys" -I"../BSW/Io" -I"../BSW/Diag" -I"../BSW/Com" -I"../Core/Inc" -I../Core/Inc -I"../BSW/Com/Can" -I"../BSW/Com/CanIf" -I"../BSW/Com/CanTp" -I"../BSW/Com/PduR" -I"../BSW/Com/UartH" -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -I"../APPL/ActCtrl" -I"../BSW" -I"../BSW/Diag/Dcm" -I"../BSW/Diag/Dem" -I"../BSW/Io/Ain" -I"../BSW/Io/TimH" -I"../BSW/Mem" -I"../BSW/Sys/EcuM" -I"../BSW/Com/CanSpi" -I"../APPL/InputCtrl" -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
clean: clean-BSW-2f-Com-2f-CanIf
clean-BSW-2f-Com-2f-CanIf:
	-$(RM) ./BSW/Com/CanIf/CanIf.o ./BSW/Com/CanIf/CanIf.d ./BSW/Com/CanIf/CanIf.su ./BSW/Com/CanIf/CanIf.cyclo
.PHONY: clean-BSW-2f-Com-2f-CanIf
