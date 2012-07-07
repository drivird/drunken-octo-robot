################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tut_solar_system.cpp \
../world2.cpp \
../world3.cpp \
../world4.cpp \
../world5.cpp \
../world6.cpp 

OBJS += \
./tut_solar_system.o \
./world2.o \
./world3.o \
./world4.o \
./world5.o \
./world6.o 

CPP_DEPS += \
./tut_solar_system.d \
./world2.d \
./world3.d \
./world4.d \
./world5.d \
./world6.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/panda3d -I/usr/include/python2.6 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


