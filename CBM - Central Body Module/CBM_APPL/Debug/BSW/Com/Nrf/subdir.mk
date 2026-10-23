################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSW/Com/Nrf/Nrf.c 

C_DEPS += \
./BSW/Com/Nrf/Nrf.d 

OBJS += \
./BSW/Com/Nrf/Nrf.o 


# Each subdirectory must supply rules for building sources it contributes
BSW/Com/Nrf/%.o BSW/Com/Nrf/%.su BSW/Com/Nrf/%.cyclo: ../BSW/Com/Nrf/%.c BSW/Com/Nrf/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I"../BSW/Com/Can" -I"../BSW/Com/CanIf" -I"../BSW/Com/CanTp" -I"../BSW/Com/PduR" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Com/UartH" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Com/Xcp" -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/APPL/ActCtrl" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Diag/Dcm" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Diag/Dem" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Io/Ain" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Io/TimH" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Mem" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Sys/EcuM" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Com/CanSpi" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/APPL/InputCtrl" -I"C:/Users/Daniel/Desktop/FG9322/CBM - Central Body Module/CBM_APPL/BSW/Com/Nrf" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSW-2f-Com-2f-Nrf

clean-BSW-2f-Com-2f-Nrf:
	-$(RM) ./BSW/Com/Nrf/Nrf.cyclo ./BSW/Com/Nrf/Nrf.d ./BSW/Com/Nrf/Nrf.o ./BSW/Com/Nrf/Nrf.su

.PHONY: clean-BSW-2f-Com-2f-Nrf

