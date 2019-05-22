#ifndef __INTEGRATION_RTOS_TEST_RUNNER_H
#define __INTEGRATION_RTOS_TEST_RUNNER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "tests/integration/FreeRTOS/nestISR.h"
#include "tests/integration/FreeRTOS/integer.h"
#include "tests/integration/FreeRTOS/dynamic_case1.h"
#include "tests/integration/FreeRTOS/dynamic_case2.h"
#include "tests/integration/FreeRTOS/block_time.h"
#include "tests/integration/FreeRTOS/queue_case1.h"
#include "tests/integration/FreeRTOS/queue_case2.h"
#include "tests/integration/FreeRTOS/queue_case3.h"
#include "tests/integration/FreeRTOS/semphr_bin_case1.h"
#include "tests/integration/FreeRTOS/semphr_bin_case2.h"

#if (configUSE_COUNTING_SEMAPHORES == 1)
    #include "tests/integration/FreeRTOS/semphr_cnt.h"
#endif // end of configUSE_COUNTING_SEMAPHORES

#if (configUSE_MUTEXES == 1)
    #include "tests/integration/FreeRTOS/mutex_case1.h"
    #if (configUSE_RECURSIVE_MUTEXES == 1)
        #include "tests/integration/FreeRTOS/recur_mutex.h"
    #endif // end of configUSE_RECURSIVE_MUTEXES
#endif // end of configUSE_MUTEXES 

#if (configUSE_TASK_NOTIFICATIONS == 1)
    #include "tests/integration/FreeRTOS/notify.h"
#endif //// end of configUSE_TASK_NOTIFICATIONS

#if (configUSE_TIMERS == 1)
    #include "tests/integration/FreeRTOS/sw_timer.h"
#endif //// end of configUSE_TIMERS

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    #include "tests/integration/FreeRTOS/stack_ovfl_chk.h"
#endif //// end of configCHECK_FOR_STACK_OVERFLOW


// ----------- function declaration -----------
void vCreateAllTestTasks( void );

void vIntegrationTestRTOSISR1(void);

void vIntegrationTestRTOSISR2(void);

BaseType_t vIntegrationTestRTOSMemManageHandler(void);

// there are few integration tests that use tick hook function,
// therefore configUSE_TICK_HOOK should be set
#if (configUSE_TICK_HOOK > 0)
void vApplicationTickHook( void );
#endif // end of configUSE_TICK_HOOK




#ifdef __cplusplus
}
#endif
#endif // end of  __INTEGRATION_RTOS_TEST_RUNNER_H

