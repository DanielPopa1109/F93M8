# Project-local source rules; regenerate from CubeIDE after configuration changes.
C_SRCS += ../Core/Src/Fbl_Identity.c \
../Core/Src/ram_blob.c \
../Core/Src/Fbl_RamFlash.c \
../Core/Src/Fbl_RamPlatform.c \
../Core/Src/can.c \
../Core/Src/Fbl.c \
../Core/Src/Fbl_Runtime.c \
../Core/Src/Fbl_Updater.c \
../Core/Src/gpio.c \
../Core/Src/main.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c

C_DEPS += ./Core/Src/Fbl_Identity.d \
./Core/Src/ram_blob.d \
./Core/Src/Fbl_RamFlash.d \
./Core/Src/Fbl_RamPlatform.d \
./Core/Src/can.d \
./Core/Src/Fbl.d \
./Core/Src/Fbl_Runtime.d \
./Core/Src/Fbl_Updater.d \
./Core/Src/gpio.d \
./Core/Src/main.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d

OBJS += ./Core/Src/Fbl_Identity.o \
./Core/Src/ram_blob.o \
./Core/Src/Fbl_RamFlash.o \
./Core/Src/Fbl_RamPlatform.o \
./Core/Src/can.o \
./Core/Src/Fbl.o \
./Core/Src/Fbl_Runtime.o \
./Core/Src/Fbl_Updater.o \
./Core/Src/gpio.o \
./Core/Src/main.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o

Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I"../BSW/Com/PduR" -I"../BSW/Com/CanTp" -I"../BSW/Com/CanIf" -I"../BSW/Com/Can" -I"../BSW/Crc" -I"../BSW/Com" -I"../BSW" -I"../Core/Inc" -I../Core/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/Daniel/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -fno-jump-tables -fno-builtin -fno-unwind-tables -fno-asynchronous-unwind-tables -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
clean: clean-Core-2f-Src
clean-Core-2f-Src:
	-$(RM) ./Core/Src/can.o ./Core/Src/can.d ./Core/Src/can.su ./Core/Src/can.cyclo ./Core/Src/Fbl.o ./Core/Src/Fbl.d ./Core/Src/Fbl.su ./Core/Src/Fbl.cyclo ./Core/Src/Fbl_Runtime.o ./Core/Src/Fbl_Runtime.d ./Core/Src/Fbl_Runtime.su ./Core/Src/Fbl_Runtime.cyclo ./Core/Src/Fbl_Updater.o ./Core/Src/Fbl_Updater.d ./Core/Src/Fbl_Updater.su ./Core/Src/Fbl_Updater.cyclo ./Core/Src/gpio.o ./Core/Src/gpio.d ./Core/Src/gpio.su ./Core/Src/gpio.cyclo ./Core/Src/main.o ./Core/Src/main.d ./Core/Src/main.su ./Core/Src/main.cyclo ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/syscalls.o ./Core/Src/syscalls.d ./Core/Src/syscalls.su ./Core/Src/syscalls.cyclo ./Core/Src/sysmem.o ./Core/Src/sysmem.d ./Core/Src/sysmem.su ./Core/Src/sysmem.cyclo ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.su ./Core/Src/system_stm32f4xx.cyclo
.PHONY: clean-Core-2f-Src
