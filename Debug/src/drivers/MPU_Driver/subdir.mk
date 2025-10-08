################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/drivers/MPU_Driver/MPU.cpp 

CPP_DEPS += \
./src/drivers/MPU_Driver/MPU.d 

OBJS += \
./src/drivers/MPU_Driver/MPU.o 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/MPU_Driver/%.o: ../src/drivers/MPU_Driver/%.cpp src/drivers/MPU_Driver/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -DDEBUG -D__CODE_RED -D__NEWLIB__ -DCORE_M0PLUS -D__MTB_BUFFER_SIZE=256 -D__USE_ROMDIVIDE -DCPP_USE_HEAP -D__LPC84X__ -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/inc" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/MPU_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/UTILS/GRAL" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/MAX_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/I2C_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/UTILS/CALLBACK" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/I2C_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/SYSTICK_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/TIMERS_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/UART_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/drivers/GPIO_Driver" -I"/Users/santiagoiamarino/Documents/MCUXpressoIDE_24.12.148/workspace/TPO-Info2/src/UTILS" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0 -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-drivers-2f-MPU_Driver

clean-src-2f-drivers-2f-MPU_Driver:
	-$(RM) ./src/drivers/MPU_Driver/MPU.d ./src/drivers/MPU_Driver/MPU.o

.PHONY: clean-src-2f-drivers-2f-MPU_Driver

