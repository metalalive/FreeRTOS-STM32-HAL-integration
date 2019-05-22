#include "tests/integration/FreeRTOS/recur_mutex.h"

#define  NUM_OF_TASKS    3
#define  MTX_REC_TAKE_MAX_COUNT  7
#define  MTX_REC_SHORT_DELAY     20
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define  intgSTACK_SIZE      (( unsigned portSHORT ) 0x3e)

// the data structure below is used only for passing parameters to the tasks in this tests
typedef struct {
    // the queue used by the tasks
    SemaphoreHandle_t    xMutex; 
    // the blocking time can be applied to Queue send/receive operations, or task delay functions
    TickType_t           xBlockTime;
    // for logging assertion failure
    TestLogger_t         *logger; 
} mtxTestParamStruct;

static volatile TaskHandle_t RecMtxMPtsk_tcb;
static volatile TaskHandle_t RecMtxHPtsk_tcb;
static unsigned portSHORT uHPtaskIsSuspended ;
static unsigned portSHORT uMPtaskIsSuspended ;



static void vRecMtxLPtsk(void *pvParams)
{
    mtxTestParamStruct *mtxParams   = (mtxTestParamStruct *) pvParams;
    SemaphoreHandle_t    xMutex     = mtxParams->xMutex; 
    TickType_t           xBlockTime = mtxParams->xBlockTime;
    TestLogger_t          *logger   = mtxParams->logger;

    BaseType_t   mtxOpsStatus;
    UBaseType_t  uxCurrentPriority;
    UBaseType_t  uxOriginPriority =  uxTaskPriorityGet( NULL );
    UBaseType_t  uxHPtskPriority  =  uxTaskPriorityGet( RecMtxHPtsk_tcb );

    for(;;)
    {
        mtxOpsStatus = xSemaphoreTakeRecursive( xMutex, xBlockTime );
        if (mtxOpsStatus == pdPASS) {
            // the HP & MP task (high-priority & medium-priority task) should be suspended at the moment
            TEST_ASSERT_EQUAL_UINT16_LOGGER( pdTRUE, uHPtaskIsSuspended, logger );
            TEST_ASSERT_EQUAL_UINT16_LOGGER( pdTRUE, uMPtaskIsSuspended, logger );
            #if ( INCLUDE_eTaskGetState == 1 )
                configASSERT( eTaskGetState(RecMtxMPtsk_tcb) == eSuspended );
                configASSERT( eTaskGetState(RecMtxHPtsk_tcb) == eSuspended );
            #endif
            // from here, this task resumes other 2 higher-priority tasks, as soon as they (the 2 tasks) are resumed
            // , they will go take this mutex again but get blocked because this (LP) task is still holding the mutex,
            // then switch back to this task (LP) .
            vTaskResume( RecMtxMPtsk_tcb );
            vTaskResume( RecMtxHPtsk_tcb );
            // when this task gets here, the 2 high-priority tasks should be blocked when they try to take the mutex
            // using xSemaphoreTakeRecursive(), this task (LP task) should inherit HP task's priority, so the priority
            // of this task is temporarily as the same as the HP task.
            uxCurrentPriority = uxTaskPriorityGet( NULL );
            TEST_ASSERT_EQUAL_UINT32_LOGGER( uxHPtskPriority, uxCurrentPriority, logger );
            // the 2 higher-priority task should be just blocked at here, not suspended .
            TEST_ASSERT_NOT_EQUAL_LOGGER( pdTRUE, uHPtaskIsSuspended, logger );
            TEST_ASSERT_NOT_EQUAL_LOGGER( pdTRUE, uMPtaskIsSuspended, logger );
            #if ( INCLUDE_eTaskGetState == 1 )
                configASSERT( eTaskGetState(RecMtxMPtsk_tcb) == eBlocked );
                configASSERT( eTaskGetState(RecMtxHPtsk_tcb) == eBlocked );
            #endif
            // release the mutex, then the give function should recover this task's priority.
            mtxOpsStatus = xSemaphoreGiveRecursive( xMutex );
            TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, mtxOpsStatus, logger );
            uxCurrentPriority = uxTaskPriorityGet( NULL );
            TEST_ASSERT_EQUAL_UINT32_LOGGER( uxOriginPriority, uxCurrentPriority, logger );
        }
        else{
            taskYIELD();
        }
    } //// end of outer infinite loop
} //// end of vRecMtxLPtsk()




static void vRecMtxMPtsk(void *pvParams)
{
    mtxTestParamStruct *mtxParams   = (mtxTestParamStruct *) pvParams;
    SemaphoreHandle_t    xMutex     = mtxParams->xMutex; 
    TickType_t           xBlockTime = mtxParams->xBlockTime;
    TestLogger_t          *logger   = mtxParams->logger;

    BaseType_t mtxOpsStatus;
    for(;;)
    {
        mtxOpsStatus = xSemaphoreTakeRecursive( xMutex, xBlockTime );
        if (mtxOpsStatus == pdPASS) {
            // the HP task (high-priority task) should be suspended at the moment
            TEST_ASSERT_EQUAL_UINT16_LOGGER( pdTRUE, uHPtaskIsSuspended, logger );
            // give the mutex , so lower priority tasks can take it.
            mtxOpsStatus = xSemaphoreGiveRecursive( xMutex );
            TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, mtxOpsStatus, logger );
            // similarly, give up CPU control to let lower priority tasks take the mutex
            uMPtaskIsSuspended = pdTRUE;
            vTaskSuspend( NULL );
            uMPtaskIsSuspended = pdFALSE;
        }
    } //// end of outer infinite loop
} //// end of vRecMtxMPtsk()




static void vRecMtxHPtsk(void *pvParams)
{
    mtxTestParamStruct *mtxParams   = (mtxTestParamStruct *) pvParams;
    SemaphoreHandle_t    xMutex     = mtxParams->xMutex; 
    TickType_t           xBlockTime = mtxParams->xBlockTime;
    TestLogger_t          *logger   = mtxParams->logger;

    BaseType_t mtxOpsStatus;
    portSHORT  idx;

    for(;;)
    {
        // a task should NOT give the mutex before it takes the mutex
        // , also if any other task alreay took the mutex but hasn't released yet ,
        //  then this task has NO right to give the mutex.
        // , therefore it cannot be successful to give the mutex.
        mtxOpsStatus = xSemaphoreGiveRecursive( xMutex );
        TEST_ASSERT_NOT_EQUAL_LOGGER( pdPASS, mtxOpsStatus, logger );
        // start taking the recursive mutex several times : 
        // when this task reaches the for-loop below at the first time, it will immediately return 
        // from xSemaphoreTakeRecursive() with pass status, the subsequent times back to the loop below
        // it will wait until the lower-priority tasks release the mutex. Therefore we need to 
        // specify short delay time to xSemaphoreTakeRecursive()
        for(idx=0; idx<MTX_REC_TAKE_MAX_COUNT; idx++) {
            mtxOpsStatus = xSemaphoreTakeRecursive( xMutex, xBlockTime );
            TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, mtxOpsStatus, logger );
            // delay this task for a while, to ensure other tasks attempting to take the mutex 
            // either get blocked or return with error . 
            vTaskDelay( xBlockTime >> 1 );
        }
        // then give the recursive mutex the same times as this task took
        for(idx=0; idx<MTX_REC_TAKE_MAX_COUNT; idx++) {
            vTaskDelay( xBlockTime >> 1 );
            mtxOpsStatus = xSemaphoreGiveRecursive( xMutex );
            TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, mtxOpsStatus, logger );
        }
        // when the task gets here, the mutex should be available again & cannot be given ,
        mtxOpsStatus = xSemaphoreGiveRecursive( xMutex );
        TEST_ASSERT_NOT_EQUAL_LOGGER( pdPASS, mtxOpsStatus, logger );
        // suspend this task, let lower priority tasks take the mutex & do something 
        uHPtaskIsSuspended = pdTRUE;
        vTaskSuspend( NULL );
        uHPtaskIsSuspended = pdFALSE;
    } //// end of outer infinite loop
} //// end of vRecMtxHPtsk




void vStartRecurMutexTest( UBaseType_t uxPriority )
{
    void (*pvTaskFuncs[NUM_OF_TASKS])(void *) = { vRecMtxLPtsk, vRecMtxMPtsk, vRecMtxHPtsk};
    const portCHAR  pcTaskName[NUM_OF_TASKS][16] = { "RecMtxLPtsk", "RecMtxMPtsk", "RecMtxHPtsk" };
    UBaseType_t     uxTaskPriorities[NUM_OF_TASKS] = { (uxPriority| portPRIVILEGE_BIT), 
                                                       ((uxPriority+1)| portPRIVILEGE_BIT), 
                                                       ((uxPriority+2)| portPRIVILEGE_BIT)  }; 
    TaskHandle_t   *xTaskHandlers[NUM_OF_TASKS] = { NULL, &RecMtxMPtsk_tcb, &RecMtxHPtsk_tcb};
    StackType_t    *stackMemSpace[NUM_OF_TASKS] ;
    BaseType_t          xState;
    unsigned portSHORT  idx;
    unsigned portSHORT  jdx;

    for (idx=0; idx<NUM_OF_TASKS; idx++) {
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc( sizeof(StackType_t) * intgSTACK_SIZE );
    }
    uHPtaskIsSuspended = pdFALSE;
    uMPtaskIsSuspended = pdFALSE;
    mtxTestParamStruct  *mtxParams[NUM_OF_TASKS];
    mtxParams[0] = (mtxTestParamStruct *) pvPortMalloc( sizeof(mtxTestParamStruct) );
    mtxParams[1] = (mtxTestParamStruct *) pvPortMalloc( sizeof(mtxTestParamStruct) );
    mtxParams[2] = (mtxTestParamStruct *) pvPortMalloc( sizeof(mtxTestParamStruct) );
    configASSERT( mtxParams[0] );
    configASSERT( mtxParams[1] );
    configASSERT( mtxParams[2] );
    // portMAX_DELAY will be considered that a blocked task never reaches time out, 
    // for MP task we use (portMAX_DELAY - 1) instead
    mtxParams[0]->xBlockTime = 0x0;
    mtxParams[1]->xBlockTime = portMAX_DELAY - 1; 
    mtxParams[2]->xBlockTime = MTX_REC_SHORT_DELAY;
    mtxParams[0]->logger = xRegisterNewTestLogger( __FILE__ , "mutex test (case 2) -- low-priority task");  
    mtxParams[1]->logger = xRegisterNewTestLogger( __FILE__ , "mutex test (case 2) -- medium-priority task");
    mtxParams[2]->logger = xRegisterNewTestLogger( __FILE__ , "mutex test (case 2) -- high-priority task"); 
    mtxParams[0]->xMutex = xSemaphoreCreateRecursiveMutex();
    mtxParams[1]->xMutex = mtxParams[0]->xMutex;
    mtxParams[2]->xMutex = mtxParams[0]->xMutex;
    configASSERT( mtxParams[0]->xMutex );

    for (idx=0; idx<NUM_OF_TASKS; idx++) 
    {
        TaskParameters_t tskparams = {
            pvTaskFuncs[idx], &pcTaskName[idx], intgSTACK_SIZE, (void *)mtxParams[ idx ],
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
} //// end of vStartRecurMutexTest()


