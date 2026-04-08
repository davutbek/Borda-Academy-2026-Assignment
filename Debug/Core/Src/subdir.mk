################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ble_simulator.c \
../Core/Src/circular_buffer.c \
../Core/Src/i2c_hal.c \
../Core/Src/main.c \
../Core/Src/median_filter.c \
../Core/Src/sensor_manager.c \
../Core/Src/sgp30.c \
../Core/Src/shtc3.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tmp102.c 

OBJS += \
./Core/Src/ble_simulator.o \
./Core/Src/circular_buffer.o \
./Core/Src/i2c_hal.o \
./Core/Src/main.o \
./Core/Src/median_filter.o \
./Core/Src/sensor_manager.o \
./Core/Src/sgp30.o \
./Core/Src/shtc3.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tmp102.o 

C_DEPS += \
./Core/Src/ble_simulator.d \
./Core/Src/circular_buffer.d \
./Core/Src/i2c_hal.d \
./Core/Src/main.d \
./Core/Src/median_filter.d \
./Core/Src/sensor_manager.d \
./Core/Src/sgp30.d \
./Core/Src/shtc3.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tmp102.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ble_simulator.cyclo ./Core/Src/ble_simulator.d ./Core/Src/ble_simulator.o ./Core/Src/ble_simulator.su ./Core/Src/circular_buffer.cyclo ./Core/Src/circular_buffer.d ./Core/Src/circular_buffer.o ./Core/Src/circular_buffer.su ./Core/Src/i2c_hal.cyclo ./Core/Src/i2c_hal.d ./Core/Src/i2c_hal.o ./Core/Src/i2c_hal.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/median_filter.cyclo ./Core/Src/median_filter.d ./Core/Src/median_filter.o ./Core/Src/median_filter.su ./Core/Src/sensor_manager.cyclo ./Core/Src/sensor_manager.d ./Core/Src/sensor_manager.o ./Core/Src/sensor_manager.su ./Core/Src/sgp30.cyclo ./Core/Src/sgp30.d ./Core/Src/sgp30.o ./Core/Src/sgp30.su ./Core/Src/shtc3.cyclo ./Core/Src/shtc3.d ./Core/Src/shtc3.o ./Core/Src/shtc3.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tmp102.cyclo ./Core/Src/tmp102.d ./Core/Src/tmp102.o ./Core/Src/tmp102.su

.PHONY: clean-Core-2f-Src

