################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../toonMakerAdvanced.cpp \
../toonMakerBasic.cpp \
../tut_cartoon_shader.cpp 

OBJS += \
./toonMakerAdvanced.o \
./toonMakerBasic.o \
./tut_cartoon_shader.o 

CPP_DEPS += \
./toonMakerAdvanced.d \
./toonMakerBasic.d \
./tut_cartoon_shader.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/panda3d -I/usr/include/python2.6 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


