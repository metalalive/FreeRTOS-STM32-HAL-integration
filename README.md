# FreeRTOS-STM32-HAL-integration

STM32CubeMX hasn't supported up-to-date version of FreeRTOS by the time I created this project, therefore I decided to get my hands dirty to work on my FreeRTOS port for STM32F4 Nucleo, which is the development board I worked with in this port.

#### Working environment
* STM32F446RE Nucleo development board, 
  * which includes ARM Cortex-M4 MCU, and useful onboard debugger.

* FreeRTOS v10.2

* C Unity
  * testing framework written in C, in this project I made modification to Unity for both of unit testing & integration testing.

* GCC toolchain
  * `arm-none-eabi-gcc` v5.4.1 20160919 (release), download the package from [here]

* OpenOCD
  * `openocd` v0.10.0, Building openOCD from source is highly recommended.

* GDB
  * `gdb-multiarch` v7.7.1.



#### Quick Start

 Options for building, running, and debugging the image

 * ```make  UNIT_TEST=yes```
   * Build image to run unit tests.

 * ```make  INTEGRATION_TEST=yes```
   * Build image to run integration tests.

 * ```make clean```
   * clean up the built image

 * ```make dbg_server  OPENOCD_HOME=/PATH/TO/YOUR_OPENOCD```
   * launch debug server, we use OpenOCD (v0.10.0) here . 
   * Note that superuser permission would be required when running openOCD, the superuser command differs & depends on your working Operating System. 

 * ```make dbg_client```
   * Before starting GDB client, please open `./test_utility.gdb` and modify the image path in your case, by going to line 73, modify `file <YOUR_PATH_TO_TEST_IMAGE>` .
   * Launch GDB client to load image, set breakpoints, watchpoints for execution. We use gdb-multiarch   (v7.7.1 or later) at here. 


#### Test Report
To see the test result, type command `report_test_result` in the GDB client console, you can see number of test cases running on the target board, and how many of them failed. 

For example, the text report below shows that we have 36 test cases and none of the tests failed.
```
$1 = "------- start of error report -------"
$2 = "------- end of error report -------"
$3 = ""
$4 = "[number of tests]:"
$5 = 36
$6 = "[number of failure]:"
$7 = 0
```

If you get some tests failed, the report also shows where did the assertion failure happen. In the case below, there is one assertion failure at line 28 of the file `sw_timer.c`, the expected value is stored in RAM address `0x200058e0`, similarly the actual value is stored in RAM address `0x200058f8`.
```
$8 = "------- start of error report -------"
$9 = "[file path]: "
$10 = 0x8009848 "Src/tests/integration/FreeRTOS/sw_timer.c"
$11 = "[line number]: "
$12 = 28
$13 = "[description]: "
$14 = 0x8009834 "software timer test"
$15 = "[expected value]: (represented as pointer) "
$16 = 0x200058e0
$17 = "[actual value]: (represented as pointer) "
$18 = 0x200058f8
$19 = ""
$20 = "------- end of error report -------"
$21 = ""
$22 = "[number of tests]:"
$23 = 36
$24 = "[number of failure]:"
$25 = 1
```


#### Code Structure

* `./Driver` : contains STM32 HAL C API functions.

* `./Inc`, `./Src` : where FreeRTOS code, testing code are located



