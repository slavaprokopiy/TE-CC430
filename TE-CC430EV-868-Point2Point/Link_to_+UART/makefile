################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/TI/ccsv5/tools/compiler/msp430_4.1.5

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./Components/bsp/bsp.obj" \
"./Components/mrfi/mrfi.obj" \
"./Components/simpliciti/nwk/nwk_globals.obj" \
"./Components/simpliciti/nwk/nwk_frame.obj" \
"./Components/simpliciti/nwk/nwk_api.obj" \
"./Components/simpliciti/nwk/nwk_QMgmt.obj" \
"./Components/simpliciti/nwk/nwk.obj" \
"./Components/simpliciti/nwk_applications/nwk_security.obj" \
"./Components/simpliciti/nwk_applications/nwk_ping.obj" \
"./Components/simpliciti/nwk_applications/nwk_mgmt.obj" \
"./Components/simpliciti/nwk_applications/nwk_link.obj" \
"./Components/simpliciti/nwk_applications/nwk_join.obj" \
"./Components/simpliciti/nwk_applications/nwk_ioctl.obj" \
"./Components/simpliciti/nwk_applications/nwk_freq.obj" \
"./uart_intfc.obj" \
"./uart.obj" \
"./radio.obj" \
"./main_LinkToWithUART.obj" \
-l"libc.a" \
"../lnk_cc430f5137.cmd" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Components/simpliciti/nwk_applications/subdir_vars.mk
-include Components/simpliciti/nwk/subdir_vars.mk
-include Components/mrfi/subdir_vars.mk
-include Components/bsp/subdir_vars.mk
-include subdir_rules.mk
-include Components/simpliciti/nwk_applications/subdir_rules.mk
-include Components/simpliciti/nwk/subdir_rules.mk
-include Components/mrfi/subdir_rules.mk
-include Components/bsp/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: TE-CC430EV-868-Point2Point.out

# Tool invocations
TE-CC430EV-868-Point2Point.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP430 Linker'
	"C:/TI/ccsv5/tools/compiler/msp430_4.1.5/bin/cl430" --cmd_file="C:\TI\workspace_v5_3\ TE-CC430EV-868-Point2Point\smpl_config.dat" --cmd_file="C:\TI\workspace_v5_3\ TE-CC430EV-868-Point2Point\smpl_nwk_config.dat"  -vmspx --abi=eabi -g --preinclude="C:/TI/SimpliciTI-CCS-1.1.1/Components/uart_options.h" --advice:power=all --define=__CC430F5137__ --define=LINK_TO --define=MRFI_CC430 --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal -z --stack_size=80 -m" TE-CC430EV-868-Point2Point.map" --heap_size=80 --use_hw_mpy=F5 -i"C:/TI/ccsv5/ccs_base/msp430/include" -i"C:/TI/ccsv5/tools/compiler/msp430_4.1.5/lib" -i"C:/TI/ccsv5/tools/compiler/msp430_4.1.5/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info=" TE-CC430EV-868-Point2Point_linkInfo.xml" --rom_model -o "TE-CC430EV-868-Point2Point.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(MSP430_EXECUTABLE_OUTPUTS__QUOTED) "TE-CC430EV-868-Point2Point.out"
	-$(RM) "main_LinkToWithUART.pp" "radio.pp" "uart.pp" "uart_intfc.pp" "Components\simpliciti\nwk_applications\nwk_freq.pp" "Components\simpliciti\nwk_applications\nwk_ioctl.pp" "Components\simpliciti\nwk_applications\nwk_join.pp" "Components\simpliciti\nwk_applications\nwk_link.pp" "Components\simpliciti\nwk_applications\nwk_mgmt.pp" "Components\simpliciti\nwk_applications\nwk_ping.pp" "Components\simpliciti\nwk_applications\nwk_security.pp" "Components\simpliciti\nwk\nwk.pp" "Components\simpliciti\nwk\nwk_QMgmt.pp" "Components\simpliciti\nwk\nwk_api.pp" "Components\simpliciti\nwk\nwk_frame.pp" "Components\simpliciti\nwk\nwk_globals.pp" "Components\mrfi\mrfi.pp" "Components\bsp\bsp.pp" 
	-$(RM) "main_LinkToWithUART.obj" "radio.obj" "uart.obj" "uart_intfc.obj" "Components\simpliciti\nwk_applications\nwk_freq.obj" "Components\simpliciti\nwk_applications\nwk_ioctl.obj" "Components\simpliciti\nwk_applications\nwk_join.obj" "Components\simpliciti\nwk_applications\nwk_link.obj" "Components\simpliciti\nwk_applications\nwk_mgmt.obj" "Components\simpliciti\nwk_applications\nwk_ping.obj" "Components\simpliciti\nwk_applications\nwk_security.obj" "Components\simpliciti\nwk\nwk.obj" "Components\simpliciti\nwk\nwk_QMgmt.obj" "Components\simpliciti\nwk\nwk_api.obj" "Components\simpliciti\nwk\nwk_frame.obj" "Components\simpliciti\nwk\nwk_globals.obj" "Components\mrfi\mrfi.obj" "Components\bsp\bsp.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
