#ifndef __BLOCK_TIME_H
#define __BLOCK_TIME_H

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

// Note if END is less than START, then overflow occurs, the interval
// should be 0xffff - START + END
#if( configUSE_16_BIT_TICKS == 1 )
    #define GET_TICKS_INTERVAL(START, END, ITVL)  \
    if(END < START) { \
        ITVL = (0xffff - START) + END ; \
    } \
    else { \
        ITVL = END - START; \
    }
#else  // otherwise we use 32-bit tick value
    #define GET_TICKS_INTERVAL(START, END, ITVL)  \
    if(END < START) { \
        ITVL = (0xffffffff - START) + END ; \
    } \
    else { \
        ITVL = END - START; \
    }
#endif 

// -----------------------------------------------------------------------------------------------
// this integration test aims at blocking time verification on tasks, caused by delay function
// , queue operations, this test can be separated into 5 parts, see description in the code for 
// each part of this test.

void vStartBlockTimeTasks( UBaseType_t uxPriority );


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __BLOCK_TIME_H

