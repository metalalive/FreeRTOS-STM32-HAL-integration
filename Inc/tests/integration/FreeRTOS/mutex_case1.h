#ifndef __MUTEX_CASE1_H
#define __MUTEX_CASE1_H

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
// this integration test aims at priority inheritance feature of FreeRTOS mutex object, 
// we create HP/MP/LP tasks  represented as high/medium/low priority tasks,
// all the tasks arrange to increase a shared counter without data corruption.
//
// the scenario goes like :
// * In the beginning, the HP/MP tasks suspend itself.
// * LP task takes mutex, then resumes the HP task.
// * HP task is blocked on the mutex taken by LP task,  then temporarily increase LP task's priority.
// * LP resumes the MP task, but LP keeps going (without preemption by MP task)
// * LP task counts up the shared counter, releases (gives) the mutex
// * HP task is resumed, get the mutex, counts up the shared counter,
//   suspend itself.
// * MP task counts up the shared counter suspend itself.

void vStartMutexTestCase1( UBaseType_t uxPriority );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __MUTEX_CASE1_H

