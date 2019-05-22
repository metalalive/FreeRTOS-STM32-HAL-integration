#ifndef __RECUR_MUTEX_H
#define __RECUR_MUTEX_H

#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"


#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------------------------------------------
// this integration test aims at how tasks access recursive mutex. Here's quick overview of recursive mutex :
// Once a task succeeds to take a recursive mutex at the first time, it is called mutex holder, the holder is
// allowed to take the same mutex several times, while other tasks, attempting to take the same mutex, 
// have to wait until the holder gives the recursive mutex as the same times as it took before.
// and thing continues the same way.
//
// There are HP, MP, LP tasks here, representing different priorities & contending for only one recursive mutex. 
// * the HP task takes the  recursive mutex several times then give the mutex the same times, 
//   after that the HP task suspends itself & RTOS switches to lower-priority task.
// * the MP task does the same thing as what HP task does, except it takes & gives the mutex only once.
// * when the LP task becomes the mutex holder, it resumes higher-priority tasks (MP/HP tasks), higher-priority
//   tasks run until they get blocked on taking the mutex, the LP task temporarily inherits HP task's priority
//   because of priority inheritance then continues running until it gives the mutex & no longer the mutex
//   holder.


void vStartRecurMutexTest( UBaseType_t uxPriority );

#ifdef __cplusplus
}  // end of extern C { ... }
#endif 
#endif // end of __RECUR_MUTEX_H

