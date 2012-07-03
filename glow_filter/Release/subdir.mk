################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../glowDemoAdvanced.cpp \
../glowDemoBasic.cpp \
../tut_glow_filter.cpp 

OBJS += \
./glowDemoAdvanced.o \
./glowDemoBasic.o \
./tut_glow_filter.o 

CPP_DEPS += \
./glowDemoAdvanced.d \
./glowDemoBasic.d \
./tut_glow_filter.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/panda3d -I/usr/include/python2.6 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


