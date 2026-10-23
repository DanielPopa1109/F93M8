################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/FreeRTOS/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/subdir.mk
-include Core/ThreadSafe/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include BSW/Sys/EcuM/subdir.mk
-include BSW/Mem/subdir.mk
-include BSW/Io/TimH/subdir.mk
-include BSW/Io/Ain/subdir.mk
-include BSW/Diag/Dem/subdir.mk
-include BSW/Diag/Dcm/subdir.mk
-include BSW/Com/UartH/subdir.mk
-include BSW/Com/CanSpi/subdir.mk
-include BSW/Com/Can/subdir.mk
-include BSW/Com/CanIf/subdir.mk
-include BSW/Com/CanTp/subdir.mk
-include BSW/Com/PduR/subdir.mk
-include BSW/Com/Com/subdir.mk
-include BSW/Com/CanNm/subdir.mk
-include BSW/Com/CanSM/subdir.mk
-include BSW/Com/ComM/subdir.mk
-include APPL/InputCtrl/subdir.mk
-include APPL/ActCtrl/subdir.mk
-include BSW/Mem/Crc/subdir.mk
-include BSW/Mem/Fls/subdir.mk
-include BSW/Mem/MemIf/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := CBM_APPL
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
CBM_APPL.elf \

MAP_FILES += \
CBM_APPL.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
CBM_APPL.list \

OBJCOPY_HEX += \
CBM_APPL.hex \

OBJCOPY_BIN += \
CBM_APPL.bin \

OBJCOPY_SREC += \
CBM_APPL.srec \


# All Target
all: main-build

# Main-build Target
main-build: CBM_APPL.elf secondary-outputs

# Tool invocations
CBM_APPL.elf CBM_APPL.map: $(OBJS) $(USER_OBJS) ../STM32F411RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "CBM_APPL.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"../STM32F411RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="CBM_APPL.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

CBM_APPL.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "CBM_APPL.list"
	@echo 'Finished building: $@'
	@echo ' '

CBM_APPL.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "CBM_APPL.hex"
	@echo 'Finished building: $@'
	@echo ' '

CBM_APPL.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "CBM_APPL.bin"
	@echo 'Finished building: $@'
	@echo ' '

CBM_APPL.srec: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O srec $(EXECUTABLES) "CBM_APPL.srec"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) CBM_APPL.bin CBM_APPL.elf CBM_APPL.hex CBM_APPL.list CBM_APPL.map CBM_APPL.srec default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN) $(OBJCOPY_SREC)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
