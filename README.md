# FreeRTOS-STM32-HAL-integration

STM32CubeMX hasn't supported up-to-date version of FreeRTOS by the time I created this project, therefore I decided to get my hands dirty to work on my FreeRTOS port for STM32F4 Nucleo, which is the development board I worked with in this port.

#### Working environment
* STM32F446RE Nucleo development board, which includes ARM Cortex-M4 MCU

* FreeRTOS v10.2

* C Unity
  testing framework written in C, in this project I made modification to Unity for both of unit testing & integration testing.

* GCC toolchain
  `arm-none-eabi-gcc` v5.4.1 20160919 (release), download the package from [here]

* OpenOCD
  `openocd` v0.10.0, Building openOCD from source is highly recommended.

* GDB
  `gdb-multiarch` v7.7.1.



#### Usage

 Options for building, running, and debugging the image

 * `make  UNIT_TEST=yes`
   Build image to run unit tests.

 * `make  INTEGRATION_TEST=yes`
   Build image to run integration tests.

 * `make dbg_server  OPENOCD_HOME=/PATH/TO/YOUR_OPENOCD`
   launch debug server, we use OpenOCD (v0.10.0) here 
   . Note that superuser permission would be required 
   when running openOCD, the command differs & depends
   on your working Operating System. 

 * `make dbg_client`  
   launch GDB client to load image, set breakpoints, 
   watchpoints for execution. We use gdb-multiarch 
   (v7.7.1 or later) at here. 

 * `make clean`
   clean up the built image



