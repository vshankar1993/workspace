################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_UPPER_SRCS += \
../cppsp_cpoll.C \
../cppsp_fcgi.C \
../page.C 

OBJS += \
./cppsp_cpoll.o \
./cppsp_fcgi.o \
./page.o 

C_UPPER_DEPS += \
./cppsp_cpoll.d \
./cppsp_fcgi.d \
./page.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.C
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../include/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

