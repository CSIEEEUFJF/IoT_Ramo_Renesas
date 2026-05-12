################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gpio.c \
../src/main.c \
../src/main_thread_entry.c \
../src/net.c \
../src/nxd_dhcp_client.c \
../src/rfid.c \
../src/storage.c \
../src/ui.c 

C_DEPS += \
./src/gpio.d \
./src/main.d \
./src/main_thread_entry.d \
./src/net.d \
./src/nxd_dhcp_client.d \
./src/rfid.d \
./src/storage.d \
./src/ui.d 

OBJS += \
./src/gpio.o \
./src/main.o \
./src/main_thread_entry.o \
./src/net.o \
./src/nxd_dhcp_client.o \
./src/rfid.o \
./src/storage.o \
./src/ui.o 

SREC += \
IoTRamoRenesas.srec 

MAP += \
IoTRamoRenesas.map 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	$(file > $@.in,-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_SYNERGY_ -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/bsp" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/driver" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/bsp" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/bsp/cmsis/Include" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/driver/api" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/driver/instances" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/src" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/src/synergy_gen" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/framework" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/api" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/instances" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/el/nxd" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/nxd" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/el/nxd_application_layer" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/tes" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy_cfg/ssp_cfg/framework/el" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/inc/framework/el" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/tx/tx_src" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/fx/fx_src" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/fx/fx_src/filex_exFAT" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/nx_md5" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/tx" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/sf_el_nx/phy" -I"C:/Users/CS/Documents/IoT_Ramo_Renesas/synergy/ssp/src/framework/el/nxd/nxd_src" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

