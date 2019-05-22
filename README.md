# FreeRTOS-STM32-HAL-integration

STM32CubeMX hasn't support up-to-date version of FreeRTOS by the time I create this project, therefore I decided to get my hands dirty to work on my FreeRTOS port for STM32F4 Nucleo, which is the development board I worked with in this port.

### Working environment in this project
* STM32F446RE Nucleo development board, which includes ARM Cortex-M4 MCU

* FreeRTOS v10.2

* C Unity
  a testing framework written in C, in this project I made modification to Unity for both of unit testing & integration testing.

* GCC toolchain
  `arm-none-eabi-gcc` v5.4.1 20160919 (release), download the package from [here]

* OpenOCD
  `openocd` v0.10.0, Building openOCD from source is highly recommended.

* GDB
  `gdb-multiarch` v7.7.1.





