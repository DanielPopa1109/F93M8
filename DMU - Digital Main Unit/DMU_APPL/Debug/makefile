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
-include BSW/Sys/FsmcH/subdir.mk
-include BSW/Sys/EcuM/subdir.mk
-include BSW/Sys/DcmiH/subdir.mk
-include BSW/Mem/Nvm/subdir.mk
-include BSW/Diag/Dem/subdir.mk
-include BSW/Diag/Dcm/subdir.mk
-include BSW/Com/I2cH/subdir.mk
-include BSW/Com/CanH/subdir.mk
-include BSW/Com/Can/subdir.mk
-include BSW/Com/CanIf/subdir.mk
-include BSW/Com/CanTp/subdir.mk
-include BSW/Com/PduR/subdir.mk
-include BSW/Com/Com/subdir.mk
-include BSW/Com/CanNm/subdir.mk
-include BSW/Com/CanSM/subdir.mk
-include BSW/Com/ComM/subdir.mk
-include APPL/DiagMonitor/subdir.mk
-include APPL/RevCam/subdir.mk
-include APPL/DigitalCluster/subdir.mk
-include APPL/DataRecorder/subdir.mk
-include BSW/Mem/subdir.mk
-include BSW/Mem/Crc/subdir.mk
-include BSW/Mem/Fls/subdir.mk
-include BSW/Mem/MemIf/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CCM_DEPS)),)
-include $(CCM_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CXXM_DEPS)),)
-include $(CXXM_DEPS)
endif
ifneq ($(strip $(C++M_DEPS)),)
-include $(C++M_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := DMU_APPL
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
DMU_APPL.elf \

MAP_FILES += \
DMU_APPL.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
DMU_APPL.list \

OBJCOPY_HEX += \
DMU_APPL.hex \

OBJCOPY_BIN += \
DMU_APPL.bin \

OBJCOPY_VERILOG += \
DMU_APPL.verilog \

OBJCOPY_SREC += \
DMU_APPL.srec \

OBJCOPY_SYMBOLSREC += \
DMU_APPL.symbolsrec \


# All Target
all: main-build

# Main-build Target
main-build: DMU_APPL.elf secondary-outputs

# Tool invocations
DMU_APPL.elf DMU_APPL.map: $(OBJS) $(USER_OBJS) ../STM32F407VETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-g++ -o "DMU_APPL.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"../STM32F407VETX_FLASH.ld" -Wl,-Map="DMU_APPL.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

DMU_APPL.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "DMU_APPL.list"
	@echo 'Finished building: $@'
	@echo ' '

DMU_APPL.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "DMU_APPL.hex"
	@echo 'Finished building: $@'
	@echo ' '

DMU_APPL.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "DMU_APPL.bin"
	@echo 'Finished building: $@'
	@echo ' '

DMU_APPL.verilog: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O verilog $(EXECUTABLES) "DMU_APPL.verilog"
	@echo 'Finished building: $@'
	@echo ' '

DMU_APPL.srec: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O srec $(EXECUTABLES) "DMU_APPL.srec"
	@echo 'Finished building: $@'
	@echo ' '

DMU_APPL.symbolsrec: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O symbolsrec $(EXECUTABLES) "DMU_APPL.symbolsrec"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) DMU_APPL.bin DMU_APPL.elf DMU_APPL.hex DMU_APPL.list DMU_APPL.map DMU_APPL.srec DMU_APPL.symbolsrec DMU_APPL.verilog default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN) $(OBJCOPY_VERILOG) $(OBJCOPY_SREC) $(OBJCOPY_SYMBOLSREC)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
