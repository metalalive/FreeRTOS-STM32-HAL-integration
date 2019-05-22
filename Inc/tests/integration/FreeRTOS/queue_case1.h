#ifndef __QUEUE_CASE1_H
#define __QUEUE_CASE1_H
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

// --------------------------------------------------------------------------------------------------------
// this integration test aims at queue operations among producer/consumer tasks with different priority,
// the priority of the tasks are determined before we start task scheduler and won't be changed dynamically
// afterwards.
//
// There are 6 tasks acting as 3 pairs of producer/consumer tasks, in this test, each pair of producer/consumer
// share the same queue . Here is summary of the scenarios :
// 
//________________________________________________________________________________________________
//   [scenario]  [task name]    [task priority]    [block time]     [queue name]    [queue size]
//________________________________________________________________________________________________
//    #1         Qks1Producer0   uxPriority         0                queue0           1
//               Qks1Consumer0   uxPriority+1       1000             queue0           1
//________________________________________________________________________________________________
//    #2         Qks1Producer1   uxPriority+1       1000             queue1           1  
//               Qks1Consumer1   uxPriority         0                queue1           1  
//________________________________________________________________________________________________
//    #3         Qks1Producer2   uxPriority         1000             queue2           5  
//               Qks1Consumer2   uxPriority         1000             queue2           5  
//________________________________________________________________________________________________
//
// In the first 2 scenarios : one of the 2 tasks has higher priority than the other,
// the higher-priority task will be blocked on a queue operation because the shared
// queue is full (in the scenario #2) or empty (in the scenario #1),
// then the lower-priority task takes over, complete its queue operation immediately without blocking time,
// and unblock high-priority task back to ready state.
// so the higher-priority task always needs some blocking time on queue operation, while
// lower-priority task doesn't need that due to the nature of the scenarios.
//
// In scenario #3, the producer and consumer has the same priority, both the 2 tasks run
// equally in round-robin manner
//

void vStartQueueTestCase1( UBaseType_t uxPriority );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __QUEUE_CASE1_H

