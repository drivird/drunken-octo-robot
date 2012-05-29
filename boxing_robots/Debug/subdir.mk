################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../robot.cpp \
../tut_boxing_robots.cpp \
../world.cpp 

OBJS += \
./robot.o \
./tut_boxing_robots.o \
./world.o 

CPP_DEPS += \
./robot.d \
./tut_boxing_robots.d \
./world.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/panda3d -I/usr/include/python2.6 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


