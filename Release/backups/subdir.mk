################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../backups/GPIO-serial\ (backup\ copy).c 

OBJS += \
./backups/GPIO-serial\ (backup\ copy).o 

C_DEPS += \
./backups/GPIO-serial\ (backup\ copy).d 


# Each subdirectory must supply rules for building sources it contributes
backups/GPIO-serial\ (backup\ copy).o: ../backups/GPIO-serial\ (backup\ copy).c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"backups/GPIO-serial (backup copy).d" -MT"backups/GPIO-serial\ (backup\ copy).d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


