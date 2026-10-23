################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APPL/ActCtrl/ActCtrl.c 

C_DEPS += \
./APPL/ActCtrl/ActCtrl.d 

OBJS += \
./APPL/ActCtrl/ActCtrl.o 


# Each subdirectory must supply rules for building sources it contributes
APPL/ActCtrl/%.o APPL/ActCtrl/%.su APPL/ActCtrl/%.cyclo: ../APPL/ActCtrl/%.c APPL/ActCtrl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"../BSW/Com/Can" -I"../BSW/Com/CanIf" -I"../BSW/Com/CanTp" -I"../BSW/Com/PduR" -I"../BSW/Com/UartH" -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -I"../APPL/ActCtrl" -I"../BSW" -I"../BSW/Diag/Dcm" -I"../BSW/Diag/Dem" -I"../BSW/Io/Ain" -I"../BSW/Io/TimH" -I"../BSW/Mem" -I"../BSW/Sys/EcuM" -I"../BSW/Com/CanSpi" -I"../APPL/InputCtrl" -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-APPL-2f-ActCtrl

clean-APPL-2f-ActCtrl:
	-$(RM) ./APPL/ActCtrl/ActCtrl.cyclo ./APPL/ActCtrl/ActCtrl.d ./APPL/ActCtrl/ActCtrl.o ./APPL/ActCtrl/ActCtrl.su

.PHONY: clean-APPL-2f-ActCtrl

