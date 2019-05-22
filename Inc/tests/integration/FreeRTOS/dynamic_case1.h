#ifndef __DYNAMIC_CASE1_H
#define __DYNAMIC_CASE1_H

#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"


#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------------------------
// This test will check how tasks interact each other with different priority, 
// A shared counter, accessed by all tasks, is used for checking functional correctness.
// There are 3 tasks at here:
//     * task 1, with priority number tskIDLE_PRIORITY. during the runtime, its priority will be 
//       temporarily increased in order to increase the shared counter by 1 unit,
//       recover its priority back to tskIDLE_PRIORITY, doing so iteratively until task 1 uses up
//       its current time slice & is forced to gives up CPU control.
//     * task 2, with priority number tskIDLE_PRIORITY + 1, In the begining this task suspends itself
//       until task 3 notify it at appropriate time, then start increasing the shared counter from
//       zero to priMAX_COUNT, then suspends itself again.
//     * task 3, with priority number tskIDLE_PRIORITY. will be seperated into 2 parts,
//       in the first part, this task checks whether the value of the shared counter modified by task 1
//       is as expected, whereas the second part checks whether the value of shared counter modified 
//       by task 2 is as expected.
//
// (Note: in FreeRTOS kernel, a task has higher priority when it gets greater natural number)
// 
void vStartDynamicPriorityCase1( UBaseType_t uxPriority );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __DYNAMIC_CASE1_H
