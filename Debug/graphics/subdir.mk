################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../graphics/camera.c \
../graphics/model.c \
../graphics/texture.c 

OBJS += \
./graphics/camera.o \
./graphics/model.o \
./graphics/texture.o 

C_DEPS += \
./graphics/camera.d \
./graphics/model.d \
./graphics/texture.d 


# Each subdirectory must supply rules for building sources it contributes
graphics/%.o: ../graphics/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


