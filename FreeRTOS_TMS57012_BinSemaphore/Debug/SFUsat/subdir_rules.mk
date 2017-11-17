################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
SFUsat/SFU_Serial.obj: ../SFUsat/SFU_Serial.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"E:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 --abi=eabi --include_path="E:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --include_path="L:/Git/Ti/HCG/LaunchProj0_TMS57012/SFUsat" --include_path="L:/Git/FreeRTOS_TMS570/FreeRTOS_TMS57012_BinSemaphore/include" -g --display_error_number --diag_wrap=off --diag_warning=225 --enum_type=packed --preproc_with_compile --preproc_dependency="SFUsat/SFU_Serial.d_raw" --obj_directory="SFUsat" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


