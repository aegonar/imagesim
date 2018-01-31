################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/imagesim.cpp 

OBJS += \
./src/imagesim.o 

CPP_DEPS += \
./src/imagesim.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -fopenmp -I/usr/local/include/opencv -I/opt/acl-3.3.0/lib_protocol/lib -I/opt/acl-3.3.0/lib_acl/lib -I/opt/acl-3.3.0/lib_acl_cpp/lib -I/opt/acl-3.3.0/dist/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


