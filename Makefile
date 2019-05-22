##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.0.0] date: [Tue Apr 16 00:40:52 CST 2019]
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = stm32_port_freertos_v10.2


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

OPENOCD_HOME=/PATH/TO/YOUR_OPENOCD_INSTALL

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
Src/stm32f4xx_it.c \
Src/stm32f4xx_hal_msp.c \
Src/stm32f4xx_hal_timebase_tim.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
Src/system_stm32f4xx.c \
Src/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
Src/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4_MPU/port.c


# ASM sources
ASM_SOURCES =  \
startup_stm32f446xx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
DUMP = $(GCC_PATH)/$(PREFIX)objdump
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
DUMP = $(PREFIX)objdump
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F446xx 



# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-IInc \
-IDrivers/STM32F4xx_HAL_Driver/Inc \
-IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
-IDrivers/CMSIS/Include \
-ISrc/Third_Party/FreeRTOS/Source/include \
-ISrc/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4_MPU \



#### ------------------------------------------------------------------
#### ---- different files & paths for unit test, integration test -----
#### ------------------------------------------------------------------
ifeq ($(UNIT_TEST), yes)
    C_SOURCES += \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_pxPortInitialiseStack.c \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_prvRestoreContextOfFirstTask.c \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_vPortPendSVHandler.c   \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_vPortSVCHandler.c      \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_xPortStartScheduler.c  \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_xPortRaisePrivilege.c  \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_vPortEnterCritical.c   \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_vPortSysTickHandler.c  \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_vPortStoreTaskMPUSettings.c  \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_vPortSuppressTicksAndSleep.c \
        Src/tests/unit/FreeRTOS/portable/ARM_CM4_MPU/test_vPortYield.c  \
        Src/Third_Party/Unity/src/unity.c \
        Src/Third_Party/Unity/extras/fixture/src/unity_fixture.c \
        Src/tests/testlogger.c \
        Src/tests/unit/FreeRTOS/test_runner.c \
        Src/tests/unit/unit_test_entry.c
    C_INCLUDES += \
        -ISrc/Third_Party/Unity/src \
        -ISrc/Third_Party/Unity/extras/fixture/src
    C_DEFS += -DUNIT_TEST
else
    ifeq ($(INTEGRATION_TEST), yes)
        C_SOURCES += \
            Src/Third_Party/FreeRTOS/Source/croutine.c      \
            Src/Third_Party/FreeRTOS/Source/event_groups.c  \
            Src/Third_Party/FreeRTOS/Source/list.c          \
            Src/Third_Party/FreeRTOS/Source/queue.c         \
            Src/Third_Party/FreeRTOS/Source/stream_buffer.c \
            Src/Third_Party/FreeRTOS/Source/tasks.c         \
            Src/Third_Party/FreeRTOS/Source/timers.c
        C_SOURCES += \
            Src/tests/testlogger.c \
            Src/Third_Party/Unity/src/unity.c \
            Src/Third_Party/Unity/extras/fixture/src/unity_fixture.c \
            Src/tests/integration/integration_test_entry.c  \
            Src/tests/integration/FreeRTOS/port/stm32f446.c \
            Src/tests/integration/FreeRTOS/nestISR.c     \
            Src/tests/integration/FreeRTOS/integer.c     \
            Src/tests/integration/FreeRTOS/dynamic_case1.c     \
            Src/tests/integration/FreeRTOS/dynamic_case2.c     \
            Src/tests/integration/FreeRTOS/block_time.c        \
            Src/tests/integration/FreeRTOS/queue_case1.c       \
            Src/tests/integration/FreeRTOS/queue_case2.c       \
            Src/tests/integration/FreeRTOS/queue_case3.c       \
            Src/tests/integration/FreeRTOS/semphr_bin_case1.c  \
            Src/tests/integration/FreeRTOS/semphr_bin_case2.c  \
            Src/tests/integration/FreeRTOS/semphr_cnt.c   \
            Src/tests/integration/FreeRTOS/mutex_case1.c  \
            Src/tests/integration/FreeRTOS/recur_mutex.c  \
            Src/tests/integration/FreeRTOS/notify.c       \
            Src/tests/integration/FreeRTOS/sw_timer.c     \
            Src/tests/integration/FreeRTOS/stack_ovfl_chk.c     \
            Src/tests/integration/FreeRTOS/test_runner.c 
        C_INCLUDES += \
            -ISrc/Third_Party/Unity/src \
            -ISrc/Third_Party/Unity/extras/fixture/src
        C_DEFS += -DINTEGRATION_TEST
    else
        C_SOURCES += Src/main.c 
    endif #### end of INTEGRATION_TEST
endif #### end of UNIT_TEST


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -Wint-to-pointer-cast

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F446RETx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).text $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR)/%.text: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(DUMP) -Dh $< > $@

$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)



#######################################
# execute/debug in the tests
#######################################
dbg_server:
	@openocd -f $(OPENOCD_HOME)/tcl/board/st_nucleo_f4.cfg \
                 -f $(OPENOCD_HOME)/tcl/interface/stlink-v2-1.cfg \
                 -c init -c "reset init"

dbg_client:
	@gdb-multiarch -x ./test_utility.gdb

#######################################
# help documentation
#######################################
help:
	@echo "                                                      ";
	@echo " ---------------- Help Documentation -----------------";
	@echo "                                                      ";
	@echo " Options for building image, running, and debugging   ";
	@echo "                                                      ";
	@echo " * make UNIT_TEST=yes                                 ";
	@echo "   Build image to run unit tests.                     ";
	@echo "                                                      ";
	@echo " * make INTEGRATION_TEST=yes                          ";
	@echo "   Build image to run integration tests.              ";
	@echo "                                                      ";
	@echo " * make dbg_server OPENOCD_HOME=/PATH/TO/YOUR_OPENOCD ";
	@echo "   launch debug server, we use OpenOCD (v0.10.0) here ";
	@echo "   . Note that superuser permission would be required ";
	@echo "   when running openOCD, the command differs & depends";
	@echo "   on your working Operating System.                  ";
	@echo "                                                      ";
	@echo " * make dbg_client                                    ";
	@echo "   launch GDB client to load image, set breakpoints,  ";
	@echo "   watchpoints for execution. We use gdb-multiarch    ";
	@echo "   (v7.7.1 or later) at here.                         ";
	@echo "                                                      ";
	@echo " * make clean                                         ";
	@echo "   clean up the built image                           ";
	@echo "                                                      ";
	@echo "                                                      ";


# *** EOF ***
