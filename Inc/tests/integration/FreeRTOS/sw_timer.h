#ifndef __SW_TIMER_TEST_H
#define __SW_TIMER_TEST_H

#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"


#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------------------------
// this integration test aims at the software timer in FreeRTOS. 
// There are 2 types of software timer defined in FreeRTOS : one-shot timer, & auto-reload timer
// , in this test we only create & verify auto-reload timer, and start it before starting RTOS
// task scheduler, then estimate tick count, compare it with the real tick count in RTOS kernel.
// 
// To work with software timer, following macros must be defined in FreeRTOSConfig.h
// see FreeRTOS Hands-On Tutorial Guide for detail.
// * configUSE_TIMERS                 <enable/disable software timer feature> 
// * configTIMER_TASK_PRIORITY        <daemon task managed by RTOS kernel, from 0 to configMAX_PRIORITIES> 
// * configTIMER_TASK_STACK_DEPTH     <daemon task managed by RTOS kernel> 
// * configTIMER_QUEUE_LENGTH         <number of items for timer command queue (managed by RTOS kernel)> 
//

void vStartSoftwareTimerTest( UBaseType_t uxPriority );



#ifdef __cplusplus
}  // end of extern C { ... }
#endif 
#endif // end of __SW_TIMER_TEST_H

