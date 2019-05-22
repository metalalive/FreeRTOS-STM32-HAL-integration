#include "tests/integration/FreeRTOS/dynamic_case2.h"

#define NUM_OF_TASKS         2
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define intgSTACK_SIZE       ( ( unsigned portSHORT ) 0x3e )

// in some cases, the following TCB_t pointers would be optimized out
// by GCC toolchain, so it is required to add volatile ahead of these pointers.
static volatile TaskHandle_t  pxDynPrAddShrCnt;
static volatile UBaseType_t   sharedCounter;


static void vDynPrChkShrCntHandler(void *pvParams)
{
    UBaseType_t  uxTask2OriginPriority = 0; 
    UBaseType_t  expectedVal = 0;
    TestLogger_t *logger = (TestLogger_t *) pvParams;

    uxTask2OriginPriority = uxTaskPriorityGet( pxDynPrAddShrCnt );

    for(;;)
    {
        // resume task 2, at this point of time, 
        // task 2 still has lower priority than this task,
        // therefore this task should NOT be preempted by task 2.
        vTaskResume( pxDynPrAddShrCnt );
        taskENTER_CRITICAL();
        sharedCounter = 0;
        expectedVal = sharedCounter;
        TEST_ASSERT_EQUAL_UINT32_LOGGER( expectedVal, sharedCounter, logger );
        taskEXIT_CRITICAL();

        // try to increase the priority of task 2 (vDynPrAddShrCntHandler) when scheduler is suspended,
        // In following lines of code, this task will be preempted after scheduler is resumed, 
        // then kernel will take control, perform context switch to task 2.
        vTaskSuspendAll();
        vTaskPrioritySet( pxDynPrAddShrCnt, (configMAX_PRIORITIES - 1) );
        taskENTER_CRITICAL();
        TEST_ASSERT_EQUAL_UINT32_LOGGER( expectedVal, sharedCounter, logger );
        taskEXIT_CRITICAL();
        xTaskResumeAll();

        // check if the shared conter was increased.
        // sharedCounter should be greater than expectedVal
        taskENTER_CRITICAL();
        expectedVal++ ;
        TEST_ASSERT_EQUAL_UINT32_LOGGER( expectedVal, sharedCounter, logger );
        taskEXIT_CRITICAL();

        vTaskDelay((TickType_t)50);
        
        // recover task 2's priority
        vTaskSuspendAll();
        vTaskPrioritySet( pxDynPrAddShrCnt, uxTask2OriginPriority );
        xTaskResumeAll();
    } //// end of outer infinite loop
} //// end of vDynPrChkShrCntHandler


static void vDynPrAddShrCntHandler(void *pvParams)
{
    for(;;)
    {
        taskENTER_CRITICAL();
        sharedCounter++;
        taskEXIT_CRITICAL();
        vTaskSuspend(NULL);
    }
}


void vStartDynamicPriorityCase2( UBaseType_t uxPriority )
{
    const UBaseType_t    taskPriority[NUM_OF_TASKS] = { 
                                                         (UBaseType_t) ((uxPriority+1) | portPRIVILEGE_BIT),
                                                         (UBaseType_t) (uxPriority | portPRIVILEGE_BIT),
                                                      };
    StackType_t     *stackMemSpace[NUM_OF_TASKS] ;
    TestLogger_t    *tlogger;
    BaseType_t       xState; 
    portSHORT        idx;

    tlogger = xRegisterNewTestLogger( __FILE__ , "dynamic priority control task (case 2)" );
    // allocate stack memory for each task
    for(idx=0; idx<NUM_OF_TASKS; idx++) {
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc(sizeof(StackType_t) * intgSTACK_SIZE );
    }
    // internally create structure to collect parameters feeding to task creating function
    TaskParameters_t tsk1params = {
        vDynPrChkShrCntHandler, "DynPrChkShrCnt", intgSTACK_SIZE, (void *) tlogger,
        taskPriority[0], stackMemSpace[0], 
        // leave MPU regions uninitialized
    }; 
    TaskParameters_t tsk2params = {
        vDynPrAddShrCntHandler, "DynPrAddShrCnt", intgSTACK_SIZE, NULL, 
        taskPriority[1], stackMemSpace[1], 
        // leave MPU regions uninitialized
    }; 
    // default value to xRegions 
    for(idx=0; idx<portNUM_CONFIGURABLE_REGIONS; idx++)
    {
        tsk1params.xRegions[idx].pvBaseAddress   = NULL;
        tsk1params.xRegions[idx].ulLengthInBytes = 0;
        tsk1params.xRegions[idx].ulParameters    = 0;
        tsk2params.xRegions[idx].pvBaseAddress   = NULL;
        tsk2params.xRegions[idx].ulLengthInBytes = 0;
        tsk2params.xRegions[idx].ulParameters    = 0;
    }

    xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tsk1params, NULL );
    configASSERT( xState == pdPASS );
    xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tsk2params, &pxDynPrAddShrCnt );
    configASSERT( xState == pdPASS );
}


