################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CIGI-serial-packets.c \
../src/rpi-mysql.c \
../src/rpi-uart.c \
../src/rpiGPIOserial.c 

OBJS += \
./src/CIGI-serial-packets.o \
./src/rpi-mysql.o \
./src/rpi-uart.o \
./src/rpiGPIOserial.o 

C_DEPS += \
./src/CIGI-serial-packets.d \
./src/rpi-mysql.d \
./src/rpi-uart.d \
./src/rpiGPIOserial.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I/home/monk/eclipse_projects/rpiGPIOserial/arm-sql-library/mysql -I"/home/monk/eclipse_projects/rpiGPIOserial/inc" -O3 -Wall -c -fmessage-length=0 -DBIG_JOINS=1  -fno-strict-aliasing   -g -lmysqlclient -lpthread -lz -lm -lrt -ldl -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


