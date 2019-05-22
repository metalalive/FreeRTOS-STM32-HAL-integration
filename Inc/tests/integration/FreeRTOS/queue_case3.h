#ifndef __QUEUE_CASE3_H
#define __QUEUE_CASE3_H
#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"


#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------------------------------------------
// this integration test aims at shared queue accessed among application tasks and interrupt service routine (ISR),
// You can use any interrupt source to implement the interrupt service routine for this test, the callback function
// for this test vQueueTestCase3ISRx()  should be implemented as producer or consumer, using appropriate function
// like xQueueSendToFrontFromISR(), xQueueSendToBackFromISR(),  xQueueReceiveFromISR() ....
// ... whatever FreeRTOS functions that end with "FromISR" should properly work in ISRs.
//
// the test can be separated into 2 parts; 
// In the first part,  task 1 shares a queue with interrupt service routine vQueueTestCase3ISR1() ,
//                    task sends items to the queue that are received in ISR.
// in the second part, task 2 shared another queue (different from first part of this test) with
//                    interrupt service routine vQueueTestCase3ISR2() ,
//                    ISR becomes sender and task becomes receiver.


void vQueueTestCase3ISR1(void);

void vQueueTestCase3ISR2(void);

void vStartQueueTestCase3( UBaseType_t uxPriority );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __QUEUE_CASE3_H

