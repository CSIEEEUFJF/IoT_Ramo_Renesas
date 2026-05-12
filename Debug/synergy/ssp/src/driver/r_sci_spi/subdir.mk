################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/driver/r_sci_spi/r_sci_spi.c 

C_DEPS += \
./synergy/ssp/src/driver/r_sci_spi/r_sci_spi.d 

OBJS += \
./synergy/ssp/src/driver/r_sci_spi/r_sci_spi.o 

SREC += \
IoTRamoRenesas.srec 

MAP += \
IoTRamoRenesas.map 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/driver/r_sci_spi/%.o: ../synergy/ssp/src/driver/r_sci_spi/%.c
	$(file > $@.in,-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_SYNERGY_ -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/bsp" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/driver" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/bsp" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/bsp/cmsis/Include" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/driver/api" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/driver/instances" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/src" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/src/synergy_gen" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/framework" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/api" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/instances" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/el/nxd" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/nxd" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/el/nxd_application_layer" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/tes" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/framework/el" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/el" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/tx/tx_src" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/fx/fx_src" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/fx/fx_src/filex_exFAT" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/nx_md5" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/tx" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/sf_el_nx/phy" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/nxd/nxd_src" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

