#ifndef __SEMPHR_CNT_H
#define __SEMPHR_CNT_H

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
// this integration test aims at the count value of counting semaphore, there are 2 tasks in this test, 
// each task accesses separate semaphore, 
// * the task continuously gives the semaphore until the count value reaches its defined maximum 
//   (think as the maximum length of a queue) ,
// * then go take the semaphore continuously until the count value reaches zero,
// then iterate the 2 steps above

void vStartCountSemphrTest( UBaseType_t uxPriority );

#ifdef __cplusplus
}  // end of extern C { ... }
#endif 
#endif // end of __SEMPHR_CNT_H

