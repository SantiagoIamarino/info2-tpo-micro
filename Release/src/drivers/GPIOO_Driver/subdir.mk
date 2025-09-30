################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/drivers/GPIOO_Driver/GPIOO.cpp 

CPP_DEPS += \
./src/drivers/GPIOO_Driver/GPIOO.d 

OBJS += \
./src/drivers/GPIOO_Driver/GPIOO.o 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/GPIOO_Driver/%.o: ../src/drivers/GPIOO_Driver/%.cpp src/drivers/GPIOO_Driver/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DNDEBUG -D__CODE_RED -D__NEWLIB__ -DCORE_M0PLUS -D__MTB_BUFFER_SIZE=256 -D__USE_ROMDIVIDE -DCPP_USE_HEAP -D__LPC84X__ -Os -fno-common -Os -g -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0 -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-drivers-2f-GPIOO_Driver

clean-src-2f-drivers-2f-GPIOO_Driver:
	-$(RM) ./src/drivers/GPIOO_Driver/GPIOO.d ./src/drivers/GPIOO_Driver/GPIOO.o

.PHONY: clean-src-2f-drivers-2f-GPIOO_Driver

