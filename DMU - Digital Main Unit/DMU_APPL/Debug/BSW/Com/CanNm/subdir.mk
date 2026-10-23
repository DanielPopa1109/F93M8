# Project-local source rules; regenerate from CubeIDE after configuration changes.
C_SRCS += ../BSW/Com/CanNm/CanNm.c

C_DEPS += ./BSW/Com/CanNm/CanNm.d

OBJS += ./BSW/Com/CanNm/CanNm.o

BSW/Com/CanNm/%.o BSW/Com/CanNm/%.su BSW/Com/CanNm/%.cyclo: ../BSW/Com/CanNm/%.c BSW/Com/CanNm/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I"../APPL" -I"../BSW/Com/PduR" -I"../BSW/Com/ComM" -I"../BSW/Com/Com" -I"../BSW/Com/CanTp" -I"../BSW/Com/CanSM" -I"../BSW/Com/CanNm" -I"../BSW/Com/CanIf" -I"../BSW/Com/Can" -I"../BSW/Mem/MemIf" -I"../BSW/Mem/Fls" -I"../BSW/Mem/Crc" -I"../BSW/Sys" -I"../BSW/Mem" -I"../BSW/Diag" -I"../BSW/Com" -I"../Core/Inc" -I../Core/Inc -I"../BSW" -I"../APPL/DigitalCluster" -I"../BSW/Sys/FsmcH" -I"../APPL/DataRecorder" -I"../APPL/RevCam" -I"../BSW/Com/CanH" -I"../BSW/Diag/Dcm" -I"../APPL/DiagMonitor" -I"../BSW/Diag/Dem" -I"../BSW/Mem/Nvm" -I"../BSW/Sys/DcmiH" -I"../BSW/Sys/EcuM" -I"../BSW/Com/I2cH" -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
clean: clean-BSW-2f-Com-2f-CanNm
clean-BSW-2f-Com-2f-CanNm:
	-$(RM) ./BSW/Com/CanNm/CanNm.o ./BSW/Com/CanNm/CanNm.d ./BSW/Com/CanNm/CanNm.su ./BSW/Com/CanNm/CanNm.cyclo
.PHONY: clean-BSW-2f-Com-2f-CanNm
