#ifndef __STK_OVFL_CHK_H
#define __STK_OVFL_CHK_H

#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"
#include "tests/integration/FreeRTOS/port/stm32f446.h"

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------------------------
// this integration test checks if the stack overflow is detected on a task.
// To run this test, be sure to add defined macro configCHECK_FOR_STACK_OVERFLOW to
// FreeRTOSConfig.h
//
// [Note]
// A more comprehensive way to recover from stack overflow is: to enable MPU, specify the valid
// address range for a task to run, when a task attempts to access an memory space that is NOT
// assigned to itself, an exception will happen, you can get more instant reply as soon as a
// task goes wrong somewhere with MPU enabled .
// 


BaseType_t  vStackOvflFaultHandler(void);


void vStartStackOverflowCheck( UBaseType_t uxPriority );



#ifdef __cplusplus
}  // end of extern C { ... }
#endif 
#endif // end of __STK_OVFL_CHK_H

