################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../gui/gui.c \
../gui/hud.c \
../gui/minimap.c 

OBJS += \
./gui/gui.o \
./gui/hud.o \
./gui/minimap.o 

C_DEPS += \
./gui/gui.d \
./gui/hud.d \
./gui/minimap.d 


# Each subdirectory must supply rules for building sources it contributes
gui/%.o: ../gui/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


