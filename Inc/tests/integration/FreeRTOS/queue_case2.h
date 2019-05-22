#ifndef __QUEUE_CASE2_H
#define __QUEUE_CASE2_H

#include <stdlib.h>
#include <math.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------------------------------------
// this integration test checks different permutations of the queue items,
// every time task 1 is responsible for feeding items in specific order to the shared queue,
// when the queue is full, the task 2 reads the item from the shared queue one after another, 
// in order to check if the permutation is correct.


void vStartQueueTestCase2( UBaseType_t uxPriority );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __QUEUE_CASE2_H

