################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSW/Com/I2cH/I2cH.c 

C_DEPS += \
./BSW/Com/I2cH/I2cH.d 

OBJS += \
./BSW/Com/I2cH/I2cH.o 


# Each subdirectory must supply rules for building sources it contributes
BSW/Com/I2cH/%.o BSW/Com/I2cH/%.su BSW/Com/I2cH/%.cyclo: ../BSW/Com/I2cH/%.c BSW/Com/I2cH/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"../BSW" -I"../APPL/DigitalCluster" -I"../BSW/Sys/FsmcH" -I"../APPL/DataRecorder" -I"../APPL/RevCam" -I"../BSW/Com/CanH" -I"../BSW/Diag/Dcm" -I"../BSW/Diag/Dem" -I"../BSW/Mem/Nvm" -I"../BSW/Sys/DcmiH" -I"../BSW/Sys/EcuM" -I"../BSW/Com/I2cH" -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSW-2f-Com-2f-I2cH

clean-BSW-2f-Com-2f-I2cH:
	-$(RM) ./BSW/Com/I2cH/I2cH.cyclo ./BSW/Com/I2cH/I2cH.d ./BSW/Com/I2cH/I2cH.o ./BSW/Com/I2cH/I2cH.su

.PHONY: clean-BSW-2f-Com-2f-I2cH

