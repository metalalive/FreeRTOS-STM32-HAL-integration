#ifndef __SEMPHR_BIN_CASE1_H
#define __SEMPHR_BIN_CASE1_H

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
// this integration test aims at binary semaphore & synchronization between 2 tasks.
// one of the 2 tasks always acts as "semaphore taker", while the other one always acts as "semaphore giver",
// Firstly taker is blocked on xSemaphoreTake() until giver gives the semaphore.
// there is a shared ariable which is counted up during the execution,
// before giver gives the semaphore, it adds some number to the shared variable, then gives the semaphore,
// then similarly taker adds some number to the shared variable, finally check the value.

void vStartBinSemphrCase1( UBaseType_t uxPriority );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __SEMPHR_BIN_CASE1_H

