################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CIGI-serial-packets.c \
../rpi-mysql.c \
../rpi-uart.c \
../rpiGPIOserial.c 

OBJS += \
./CIGI-serial-packets.o \
./rpi-mysql.o \
./rpi-uart.o \
./rpiGPIOserial.o 

C_DEPS += \
./CIGI-serial-packets.d \
./rpi-mysql.d \
./rpi-uart.d \
./rpiGPIOserial.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I/home/monk/eclipse_projects/rpiGPIOserial/arm-sql-library/mysql -I"/home/monk/eclipse_projects/rpiGPIOserial/inc" -O3 -Wall -c -fmessage-length=0 -DBIG_JOINS=1  -fno-strict-aliasing   -g -lmysqlclient -lpthread -lz -lm -lrt -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


