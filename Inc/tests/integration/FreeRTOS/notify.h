#ifndef __NOTIFY_TEST_H
#define __NOTIFY_TEST_H

#include <stdlib.h>
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#include "task.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"


#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------------------------
// this integration test aims at task notification between tasks, or between task and ISR.
//
// * In the first part of this test, a taker task waits until it receives notification event from
//   the giver task, the 2 tasks take turn to count up the shared variable, and check the value .
//   task notification in this part can be considered as binary semaphore, working among the 2 tasks
//   , since we assign pdTRUE to the first argument of ulTaskNotifyTake(). See FreeRTOS API Reference
//   Manual for detail.
//
// * In the second part, an ISR acts to generate task notification event to a task which might be 
//   blocked on ulTaskNotifyTake(). if the interrupt generates task notification event very quickly ,
//   then the task might not be able to instantly process every notification event once it is generated.
//   To avoid the task loses notification events which have been ready but not processed yet, 
//   we assign pdFALSE to the first argument of ulTaskNotifyTake(), so  task notification in this part
//   can be considered as counting semaphore, a buffer for events waiting to be processed.
//

void vStartNotifyTaskTest( UBaseType_t uxPriority );

void vNotifyTestISR( void );


#ifdef __cplusplus
}  // end of extern C { ... }
#endif 
#endif // end of __NOTIFY_TEST_H

