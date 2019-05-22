#ifndef __SEMPHR_BIN_CASE2_H
#define __SEMPHR_BIN_CASE2_H

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
// this integration test aims at binary semaphore & synchronization between a task and interrupt.
// we build a typical scenario that an interrupt acts as "semaphore giver", which will give semaphore to the taker :
// the task in this test. Once the taker gets semaphore, it will change the value of shared variable, 
//

void vStartBinSemphrCase2( UBaseType_t uxPriority );

void vBinSemphrCase2ISR1( void );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __SEMPHR_BIN_CASE2_H

