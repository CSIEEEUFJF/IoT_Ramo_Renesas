################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d.c \
../synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_8bit_palette.c \
../synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_8bit_palette_rotated.c \
../synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_rotated.c \
../synergy/ssp/src/framework/sf_el_gx/sf_el_gx.c 

C_DEPS += \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d.d \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_8bit_palette.d \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_8bit_palette_rotated.d \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_rotated.d \
./synergy/ssp/src/framework/sf_el_gx/sf_el_gx.d 

OBJS += \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d.o \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_8bit_palette.o \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_8bit_palette_rotated.o \
./synergy/ssp/src/framework/sf_el_gx/gx_display_driver_synergy_dave2d_rotated.o \
./synergy/ssp/src/framework/sf_el_gx/sf_el_gx.o 

SREC += \
PortaLesGO.srec 

MAP += \
PortaLesGO.map 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/framework/sf_el_gx/%.o: ../synergy/ssp/src/framework/sf_el_gx/%.c
	$(file > $@.in,-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_SYNERGY_ -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/bsp" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/driver" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/bsp" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/bsp/cmsis/Include" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/driver/api" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/driver/instances" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/src" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/src/synergy_gen" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/framework/el" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/el" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/src/framework/el/tx" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/framework" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/api" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/instances" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/tes" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp_supplemental/inc/framework/instances" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

