################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../graphics/world/baloon.c \
../graphics/world/billboard.c \
../graphics/world/copter.c \
../graphics/world/crate.c \
../graphics/world/sky.c \
../graphics/world/terrain.c 

OBJS += \
./graphics/world/baloon.o \
./graphics/world/billboard.o \
./graphics/world/copter.o \
./graphics/world/crate.o \
./graphics/world/sky.o \
./graphics/world/terrain.o 

C_DEPS += \
./graphics/world/baloon.d \
./graphics/world/billboard.d \
./graphics/world/copter.d \
./graphics/world/crate.d \
./graphics/world/sky.d \
./graphics/world/terrain.d 


# Each subdirectory must supply rules for building sources it contributes
graphics/world/%.o: ../graphics/world/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


