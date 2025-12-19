################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/blinky_thread_entry.c \
../src/hal_entry.c \
../src/new_thread0_entry.c 

C_DEPS += \
./src/blinky_thread_entry.d \
./src/hal_entry.d \
./src/new_thread0_entry.d 

OBJS += \
./src/blinky_thread_entry.o \
./src/hal_entry.o \
./src/new_thread0_entry.o 

SREC += \
PortaLesGO.srec 

MAP += \
PortaLesGO.map 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	$(file > $@.in,-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_SYNERGY_ -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/bsp" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/driver" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/bsp" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/bsp/cmsis/Include" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/driver/api" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/driver/instances" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/src" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/src/synergy_gen" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/framework/el" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/el" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/src/framework/el/tx" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy_cfg/ssp_cfg/framework" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/api" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/instances" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp/inc/framework/tes" -I"C:/Users/Rafael/e2_studio/workspace/PortaLesGO/synergy/ssp_supplemental/inc/framework/instances" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

