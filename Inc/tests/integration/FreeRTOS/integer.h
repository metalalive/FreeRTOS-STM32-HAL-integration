#ifndef __INTEGER_H
#define __INTEGER_H

#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------------------------
// This is stability test of integer arithmetic operations
// , to see if the calculation will go wrong after context switches.
// There are 4 tasks, first 2 of them perform the same arithmetic sequence, 
// the last 2 tasks perform summation of the same array, which is created 
// using pvPortMalloc().


// the only entry to start these (sub) tasks performing integer operations
void vStartIntegerMathTasks( UBaseType_t uxPriority );

#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __INTEGER_H

