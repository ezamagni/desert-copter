################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include models/crate/subdir.mk
-include models/copter/subdir.mk
-include models/billboard/subdir.mk
-include models/baloon/subdir.mk
-include gui/subdir.mk
-include graphics/world/subdir.mk
-include graphics/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: DesertCopter

# Tool invocations
DesertCopter: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L/usr/X11R6 -o "DesertCopter" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) DesertCopter
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
