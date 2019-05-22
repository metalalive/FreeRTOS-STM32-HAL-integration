#include "tests/integration/FreeRTOS/notify.h"

#define  NUM_OF_TASKS    3
#define  NOTIFY_MAX_NUM_EVENTS  100
#define  TEST_ADD_TEN      10
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define  intgSTACK_SIZE      (( unsigned portSHORT ) 0x3e)

// the data structure below is used only for passing parameters to the tasks in this tests
typedef struct {
    // shared variables used in this test
    volatile UBaseType_t *pulSharedVariable;
    // for logging assertion failure
    TestLogger_t         *logger; 
} notiParamStruct;


static volatile TaskHandle_t NtfT2Ttaker_tcb;
static volatile TaskHandle_t NtfI2Ttaker_tcb;
static volatile UBaseType_t *pulSharedVariable_ISR1;


static void vNotifyTsk2tskTaker(void *pvParams)
{
    notiParamStruct  *notiParams   = (notiParamStruct *) pvParams;
    UBaseType_t *pulSharedVariable = notiParams->pulSharedVariable ;
    // for inserting error flags
    TestLogger_t         *logger   = notiParams->logger;

    uint32_t ulTskNotifyReturn ;
    for(;;)
    {
        *pulSharedVariable = 0x0;
        // the first argument of following function is used to indicate whether to 
        // reset the internal nortification value (of current task) to zero on exiting
        // this function, 
        // By passing pdTRUE  to the first argument, the notification value is always set 
        // to zero on exiting the take function, This can be used in place of binary semaphore. 
        ulTskNotifyReturn = ulTaskNotifyTake( pdTRUE, (portMAX_DELAY - 1) );
        TEST_ASSERT_EQUAL_UINT32_LOGGER( TEST_ADD_TEN, (*pulSharedVariable), logger );
        *pulSharedVariable += TEST_ADD_TEN;
        // wait for notification again
        ulTskNotifyReturn = ulTaskNotifyTake( pdTRUE, (portMAX_DELAY - 1) );
        TEST_ASSERT_EQUAL_UINT32_LOGGER( (TEST_ADD_TEN*3), (*pulSharedVariable), logger );
    }
} //// end of vNotifyTsk2tskTaker()




static void vNotifyTsk2tskGiver(void *pvParams)
{
    notiParamStruct    *notiParams = (notiParamStruct *) pvParams;
    UBaseType_t *pulSharedVariable = notiParams->pulSharedVariable ;
    // for inserting error flags
    TestLogger_t         *logger   = notiParams->logger;
    portSHORT idx = 0;

    for(;;)
    {
        *pulSharedVariable += TEST_ADD_TEN;
        taskYIELD();
        xTaskNotifyGive( NtfT2Ttaker_tcb );
        TEST_ASSERT_EQUAL_UINT32_LOGGER( (TEST_ADD_TEN*2), (*pulSharedVariable), logger );
        *pulSharedVariable += TEST_ADD_TEN;
        taskYIELD();
        xTaskNotifyGive( NtfT2Ttaker_tcb );
    }
} //// end of vNotifyTsk2tskGiver()




void vNotifyTestISR( void )
{
    BaseType_t  pxHigherPriorityTaskWoken = pdFALSE;
    // eveny time the interrupt happens , the ISR increments the shared variable 
    // (shared with taker task) until the shared variable reaches the defined maximum,
    // then stop incrementing until the taker task resets the shared variable to zero
    // . See how it works with vNotifyISR2TskTaker()
    if ((*pulSharedVariable_ISR1) < NOTIFY_MAX_NUM_EVENTS)
    {
        (*pulSharedVariable_ISR1) += 1;
        vTaskNotifyGiveFromISR( NtfI2Ttaker_tcb, &pxHigherPriorityTaskWoken );
        portYIELD_FROM_ISR( pxHigherPriorityTaskWoken );
    }
} //// end of vNotifyTestISR1()




static void vNotifyISR2TskTaker(void *pvParams)
{
    notiParamStruct  *notiParams = (notiParamStruct *) pvParams;
    volatile UBaseType_t *pulSharedVariable = notiParams->pulSharedVariable ;
    // for inserting error flags
    TestLogger_t          *logger   = notiParams->logger;
    // number of notification events which are processed, this variable is reset
    // as soon as this task has processed NOTIFY_MAX_NUM_EVENTS notification events .
    uint32_t  ulNumNotifyEvtProcessed;
    // ulTaskNotifyTake() returns the last internal notification value before the value is internally changed
    uint32_t  uLastNotifyValB4Decrement;

    ulNumNotifyEvtProcessed   = 0;
    uLastNotifyValB4Decrement = 0;

    for (;;)
    {
        // By passing pdFALSE to the first argument, then this function simply passes the internal 
        // notification value as the return variable before it is decremented by one, then decrement
        // the notification value, return it on exit,  
        // this is useful for those applications which notification event is generated (e.g. by interrupt)
        // very fast but the receiving task cannot instantly handle all these events,
        // in such scenario, the internal notification value of a task works like a buffer for these
        // events which are ready but have not been processed yet. 
        // Therefore to pass pdFALSE to the first argument, this can be considered as what counting
        // semaphore does 
        uLastNotifyValB4Decrement = ulTaskNotifyTake( pdFALSE, (portMAX_DELAY - 1) );
        ulNumNotifyEvtProcessed += 1;
        // if the number of processed events currently reached maximum ...
        if(ulNumNotifyEvtProcessed == NOTIFY_MAX_NUM_EVENTS)
        {
            TEST_ASSERT_EQUAL_UINT32_LOGGER( 1, uLastNotifyValB4Decrement, logger );
            // check if we process the same number of notification signals from the interrupt
            TEST_ASSERT_EQUAL_UINT32_LOGGER( ulNumNotifyEvtProcessed, (*pulSharedVariable), logger );
            // optional delay to let tasks of other tests take CPU control.
            vTaskDelay( pdMS_TO_TICKS(60) );
            // to disable interrupts whose "priority number" is greater than configMAX_SYSCALL_INTERRUPT_PRIORITY 
            // , we can avoid interrupt in vNotifyTestISR() corrupts pulSharedVariable at here.
            taskENTER_CRITICAL();
            {
                *pulSharedVariable = 0;
                ulNumNotifyEvtProcessed = 0;
            }
            taskEXIT_CRITICAL();
        }
    } //// end of outer infinite loop
} //// end of vNotifyISR2TskTaker()





void vStartNotifyTaskTest( UBaseType_t uxPriority )
{
    void (*pvTaskFuncs[NUM_OF_TASKS])(void *) = { vNotifyTsk2tskTaker, vNotifyTsk2tskGiver, vNotifyISR2TskTaker};
    const portCHAR pcTaskName[NUM_OF_TASKS][16] = { "NtfT2Ttaker", "NtfT2Tgiver", "NtfI2Ttaker"};
    UBaseType_t  uxTaskPriorities[NUM_OF_TASKS] = { ((uxPriority+1) | portPRIVILEGE_BIT), 
                                                    (uxPriority     | portPRIVILEGE_BIT), 
                                                    ((uxPriority+1) | portPRIVILEGE_BIT)  }; 
    TaskHandle_t   *xTaskHandlers[NUM_OF_TASKS] = { &NtfT2Ttaker_tcb, NULL, &NtfI2Ttaker_tcb };
    StackType_t    *stackMemSpace[NUM_OF_TASKS] ;
    BaseType_t          xState;
    unsigned portSHORT  idx;
    unsigned portSHORT  jdx;

    for (idx=0; idx<NUM_OF_TASKS; idx++) {
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc( sizeof(StackType_t) * intgSTACK_SIZE );
    }
    notiParamStruct  *notiParams[NUM_OF_TASKS];
    notiParams[0] = (notiParamStruct *) pvPortMalloc( sizeof(notiParamStruct) );
    notiParams[1] = (notiParamStruct *) pvPortMalloc( sizeof(notiParamStruct) );
    notiParams[2] = (notiParamStruct *) pvPortMalloc( sizeof(notiParamStruct) );
    configASSERT( notiParams[0] );
    configASSERT( notiParams[1] );
    configASSERT( notiParams[2] );
    notiParams[0]->pulSharedVariable = (UBaseType_t *) pvPortMalloc( sizeof(UBaseType_t) );
    notiParams[1]->pulSharedVariable = notiParams[0]->pulSharedVariable;
    notiParams[2]->pulSharedVariable = (UBaseType_t *) pvPortMalloc( sizeof(UBaseType_t) );
    pulSharedVariable_ISR1           = notiParams[2]->pulSharedVariable;
    configASSERT( notiParams[0]->pulSharedVariable );
    configASSERT( notiParams[2]->pulSharedVariable );
    *( notiParams[0]->pulSharedVariable ) = 0x0;
    *( notiParams[2]->pulSharedVariable ) = 0x0;
    notiParams[0]->logger = xRegisterNewTestLogger( __FILE__ , "notification test -- task-to-task, taker");
    notiParams[1]->logger = xRegisterNewTestLogger( __FILE__ , "notification test -- task-to-task, giver");
    notiParams[2]->logger = xRegisterNewTestLogger( __FILE__ , "notification test -- task-to-ISR, taker");
    for (idx=0; idx<NUM_OF_TASKS; idx++) 
    {
        TaskParameters_t tskparams = {
            pvTaskFuncs[idx], &pcTaskName[idx], intgSTACK_SIZE, (void *)notiParams[ idx ],
            uxTaskPriorities[idx], stackMemSpace[idx],
            // leave MPU regions uninitialized
        };
        // default value to unused MPU regions 
        for(jdx=0; jdx<portNUM_CONFIGURABLE_REGIONS; jdx++)
        {
            tskparams.xRegions[jdx].pvBaseAddress   = NULL;
            tskparams.xRegions[jdx].ulLengthInBytes = 0;
            tskparams.xRegions[jdx].ulParameters    = 0;
        }
        xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tskparams, xTaskHandlers[idx] );
        configASSERT( xState == pdPASS );
    }

} //// end of vStartNotifyTaskTest()

