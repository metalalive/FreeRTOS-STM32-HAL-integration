#ifndef __DYNAMIC_CASE2_H
#define __DYNAMIC_CASE2_H

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
// as how the test dynamic_case1 works.
// 
// A shared counter, accessed by all tasks, is used for checking functional correctness.
// There are 2 tasks at here:
//
//     * task 1: iteratively monitor the value of the shared counter, control task 2's state
//               , e.g. suspend/resume task 2, increase/decrease priority of task 2
// 
//     * task 2: increase the shared counter by 1 unit, then suspend itself
//

void vStartDynamicPriorityCase2( UBaseType_t uxPriority );

#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __DYNAMIC_CASE2_H

