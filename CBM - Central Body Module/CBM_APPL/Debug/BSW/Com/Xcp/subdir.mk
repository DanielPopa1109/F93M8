################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSW/Com/Xcp/Xcp.c 

C_DEPS += \
./BSW/Com/Xcp/Xcp.d 

OBJS += \
./BSW/Com/Xcp/Xcp.o 


# Each subdirectory must supply rules for building sources it contributes
BSW/Com/Xcp/%.o BSW/Com/Xcp/%.su BSW/Com/Xcp/%.cyclo: ../BSW/Com/Xcp/%.c BSW/Com/Xcp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"../BSW/Com/Can" -I"../BSW/Com/CanIf" -I"../BSW/Com/CanTp" -I"../BSW/Com/PduR" -I"../BSW/Com/UartH" -I"../BSW/Com/Xcp" -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -I"../APPL/ActCtrl" -I"../BSW" -I"../BSW/Diag/Dcm" -I"../BSW/Diag/Dem" -I"../BSW/Io/Ain" -I"../BSW/Io/TimH" -I"../BSW/Mem" -I"../BSW/Sys/EcuM" -I"../BSW/Com/CanSpi" -I"../APPL/InputCtrl" -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSW-2f-Com-2f-Xcp

clean-BSW-2f-Com-2f-Xcp:
	-$(RM) ./BSW/Com/Xcp/Xcp.cyclo ./BSW/Com/Xcp/Xcp.d ./BSW/Com/Xcp/Xcp.o ./BSW/Com/Xcp/Xcp.su

.PHONY: clean-BSW-2f-Com-2f-Xcp

