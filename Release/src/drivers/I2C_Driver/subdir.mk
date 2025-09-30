################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/drivers/I2C_Driver/I2C.cpp 

CPP_DEPS += \
./src/drivers/I2C_Driver/I2C.d 

OBJS += \
./src/drivers/I2C_Driver/I2C.o 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/I2C_Driver/%.o: ../src/drivers/I2C_Driver/%.cpp src/drivers/I2C_Driver/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DNDEBUG -D__CODE_RED -D__NEWLIB__ -DCORE_M0PLUS -D__MTB_BUFFER_SIZE=256 -D__USE_ROMDIVIDE -DCPP_USE_HEAP -D__LPC84X__ -Os -fno-common -Os -g -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0 -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-drivers-2f-I2C_Driver

clean-src-2f-drivers-2f-I2C_Driver:
	-$(RM) ./src/drivers/I2C_Driver/I2C.d ./src/drivers/I2C_Driver/I2C.o

.PHONY: clean-src-2f-drivers-2f-I2C_Driver

