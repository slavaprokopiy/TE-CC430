################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Components/bsp/bsp.obj: C:/TI/SimpliciTI-CCS-1.1.1/Components/bsp/bsp.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/TI/ccsv5/tools/compiler/msp430_4.1.5/bin/cl430" --cmd_file="C:\TI\workspace_v5_3\ TE-CC430EV-868-Star\smpl_nwk_config.dat" --cmd_file="C:\TI\workspace_v5_3\ TE-CC430EV-868-Star\AP_smpl_config.dat"  -vmspx --abi=eabi -g --preinclude="C:/TI/SimpliciTI-CCS-1.1.1/Components/uart_options.h" --include_path="C:/TI/ccsv5/ccs_base/msp430/include" --include_path="C:/TI/SimpliciTI-CCS-1.1.1/Components/bsp" --include_path="C:/TI/SimpliciTI-CCS-1.1.1/Components/bsp/boards/430BOOST-CC430" --include_path="C:/TI/SimpliciTI-CCS-1.1.1/Components/bsp/drivers" --include_path="C:/TI/SimpliciTI-CCS-1.1.1/Components/mrfi" --include_path="C:/TI/SimpliciTI-CCS-1.1.1/Components/simpliciti/nwk" --include_path="C:/TI/SimpliciTI-CCS-1.1.1/Components/simpliciti/nwk_applications" --include_path="C:/TI/ccsv5/tools/compiler/msp430_4.1.5/include" --advice:power=all --define=__CC430F5137__ --define=I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE --define=MRFI_CC430 --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="Components/bsp/bsp.pp" --obj_directory="Components/bsp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

