#include "tests/integration/FreeRTOS/mutex_case1.h"

#define  NUM_OF_TASKS  3
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define  intgSTACK_SIZE      (( unsigned portSHORT ) 0x3e)
#define  TEST_ADD_TWO    2
#define  TEST_ADD_FIVE   5
#define  TEST_ADD_THREE  3

// the data structure below is used only for passing parameters to the tasks in this tests
typedef struct {
    // the queue used by the tasks
    SemaphoreHandle_t    xMutex; 
    // the blocking time can be applied to Queue send/receive operations, or task delay functions
    TickType_t           xBlockTime;
    // for logging assertion failure
    TestLogger_t         *logger; 
} mtxTestParamStruct;

// TCB structure of medium-priority task and high-priority task
static volatile TaskHandle_t xMtxKs1MPtsk_tcb;
static volatile TaskHandle_t xMtxKs1HPtsk_tcb;
// shared counters used only in this test
static portSHORT sharedCounter ;


static void vMtxKs1AccessMultiMutex1( mtxTestParamStruct *mtxParams )
{
    // This function should always be executed by the low-priority task.
    // The task running this function will take/give the 2 mutexes in following order:
    //     #1:  take the input mutex
    //     #2:  take the local mutex
    //     #3:  give the local mutex
    //     #4:  give the input mutex
    // then checks its priority and the shared counter.
    SemaphoreHandle_t    xMutexIn = mtxParams->xMutex;
    TickType_t         xBlockTime = mtxParams->xBlockTime;
    TestLogger_t          *logger = mtxParams->logger;
    const TickType_t   xDontBlock = 0;
    // second mutex used only in this function
    SemaphoreHandle_t  xMutexLocal = NULL ;
    // expected value of shared counter after all the tasks count up the counter.
    UBaseType_t expectedCounterValue = TEST_ADD_TWO + TEST_ADD_FIVE + TEST_ADD_THREE;

    BaseType_t mtxOpsStatus;
    UBaseType_t uxPriorityLPtsk;
    UBaseType_t uxPriorityHPtsk;
    UBaseType_t uxCurrentPriority;

    // set shared counter to zero
    sharedCounter = 0;
    // status returned from a mutex operation
    xMutexLocal = xSemaphoreCreateMutex();
    configASSERT( xMutexLocal );
    // record the priority of this task (LP task) and HP task before they take the mutex
    uxPriorityLPtsk = uxTaskPriorityGet( NULL );
    uxPriorityHPtsk = uxTaskPriorityGet( xMtxKs1HPtsk_tcb );

    // #1: take the input mutex
    mtxOpsStatus = xSemaphoreTake( xMutexIn, xDontBlock);
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, mtxOpsStatus, logger );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityLPtsk, uxCurrentPriority, logger );
    // wake up the high-priority task & perform context switch, 
    // the HP task will take the same mutex xMutexIn but result in blocked state
    vTaskResume( xMtxKs1HPtsk_tcb );
    // when the LP task gets here, the HP task should be blocked because it fails to take xMutexIn, 
    // then priority inheritance should work to temporarily increase this task's priority
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_NOT_EQUAL_LOGGER(    uxPriorityLPtsk, uxCurrentPriority, logger );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityHPtsk, uxCurrentPriority, logger );
    // try modifying LP task's priority, its priority should not be changed at here because 
    // the LP task hasn't released xMutexIn and still inherits HP task's priority.
    vTaskPrioritySet( NULL, uxPriorityLPtsk + 1 );
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityHPtsk, uxCurrentPriority, logger );
    // resume MP task, but MP should NOT immediately preempt this (LP) task because of priority inheritance.
    vTaskResume( xMtxKs1MPtsk_tcb );
    TEST_ASSERT_EQUAL_INT16_LOGGER( 0, sharedCounter, logger );

    // #2: take the local mutex
    mtxOpsStatus = xSemaphoreTake( xMutexLocal, xDontBlock);
    TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, mtxOpsStatus, logger );

    // modify the shared counter
    sharedCounter += TEST_ADD_TWO;

    // #3: give the local mutex
    mtxOpsStatus = xSemaphoreGive( xMutexLocal );
    // LP task's priority still remains the same, equal to HP task's priority
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityHPtsk, uxCurrentPriority, logger );
    // LP task's still inherits HP task's priority at here, so there shouldn't be context switch since last time
    // LP task resumed HP task.
    TEST_ASSERT_EQUAL_INT16_LOGGER( TEST_ADD_TWO, sharedCounter, logger );

    // #4: give the input mutex, 
    // from here the HP task gets xMutexIn then immeidately preempt LP task,
    mtxOpsStatus = xSemaphoreGive( xMutexIn );
    TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, mtxOpsStatus, logger );

    // when LP task gets here, its priority should no longer be the same as the HP task.
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( (uxPriorityLPtsk+1), uxCurrentPriority, logger );
    // before this task, the LP task already added TEST_ADD_TWO to the shared counter.
    // here we simply recheck the value of the shared counter
    TEST_ASSERT_EQUAL_INT16_LOGGER( expectedCounterValue, sharedCounter, logger );

    // recover LP task's priority
    vTaskPrioritySet( NULL, uxPriorityLPtsk );
    // don't forget to delete the local mutex to avoid memory leak
    vSemaphoreDelete( xMutexLocal );
    xMutexLocal = NULL;
} //// end of vMtxKs1AccessMultiMutex1()





static void vMtxKs1AccessMultiMutex2( mtxTestParamStruct *mtxParams )
{
    // This function should always be executed by the low-priority task.
    // The task running this function will take/give the 2 mutexes in following order:
    //     #1:  take the input mutex
    //     #2:  take the local mutex
    //     #3:  give the input mutex
    //     #4:  give the local mutex
    // the order of releasing the mutex is different from vMtxKs1AccessMultiMutex1()
    // then checks its priority and the shared counter.
    SemaphoreHandle_t    xMutexIn = mtxParams->xMutex;
    TickType_t         xBlockTime = mtxParams->xBlockTime;
    TestLogger_t          *logger = mtxParams->logger;
    const TickType_t   xDontBlock = 0;
    // second mutex used only in this function
    SemaphoreHandle_t  xMutexLocal = NULL ;
    // expected value of shared counter after all the tasks count up the counter.
    UBaseType_t expectedCounterValue = TEST_ADD_TWO + TEST_ADD_FIVE + TEST_ADD_THREE;

    BaseType_t mtxOpsStatus;
    UBaseType_t uxPriorityLPtsk;
    UBaseType_t uxPriorityHPtsk;
    UBaseType_t uxCurrentPriority;

    // set shared counter to zero
    sharedCounter = 0;
    // status returned from a mutex operation
    xMutexLocal = xSemaphoreCreateMutex();
    configASSERT( xMutexLocal );
    // record the priority of this task (LP task) and HP task before they take the mutex
    uxPriorityLPtsk = uxTaskPriorityGet( NULL );
    uxPriorityHPtsk = uxTaskPriorityGet( xMtxKs1HPtsk_tcb );

    // #1: ---------------- take the input mutex ----------------
    mtxOpsStatus = xSemaphoreTake( xMutexIn, xDontBlock);
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, mtxOpsStatus, logger );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityLPtsk, uxCurrentPriority, logger );
    // wake up the high-priority task & perform context switch, 
    // the HP task will take the same mutex xMutexIn but result in blocked state
    vTaskResume( xMtxKs1HPtsk_tcb );
    // when the LP task gets here, the HP task should be blocked because it fails to take xMutexIn, 
    // then priority inheritance should work to temporarily increase this task's priority
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_NOT_EQUAL_LOGGER(    uxPriorityLPtsk, uxCurrentPriority, logger );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityHPtsk, uxCurrentPriority, logger );
    // try modifying LP task's priority, its priority should not be changed at here because 
    // the LP task hasn't released xMutexIn and still inherits HP task's priority.
    vTaskPrioritySet( NULL, uxPriorityLPtsk + 1 );
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityHPtsk, uxCurrentPriority, logger );
    // resume MP task, but MP should NOT immediately preempt this (LP) task because of priority inheritance.
    vTaskResume( xMtxKs1MPtsk_tcb );
    TEST_ASSERT_EQUAL_INT16_LOGGER( 0, sharedCounter, logger );

    // #2: ---------------- take the local mutex ----------------
    mtxOpsStatus = xSemaphoreTake( xMutexLocal, xDontBlock);
    TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, mtxOpsStatus, logger );

    // modify the shared counter
    sharedCounter += TEST_ADD_TWO;

    // #3: ---------------- give the input mutex ----------------
    mtxOpsStatus = xSemaphoreGive( xMutexIn );
    // LP task's priority still remains the same, equal to HP task's priority,
    // even LP task already released xMutexIn HP task waits for, LP task is still NOT preempted by HP task,
    // It is because of the limit of FreeRTOS implementation, in such case, LP task needs to give all the mutexes
    // it took before HP task can preempt LP task. See the archived discussion below :
    // https://www.freertos.org/FreeRTOS_Support_Forum_Archive/June_2017/freertos_Nested_mutexes_and_priority_inheritance_again_30f4e9ebj.html
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( uxPriorityHPtsk, uxCurrentPriority, logger );
    TEST_ASSERT_EQUAL_INT16_LOGGER( TEST_ADD_TWO, sharedCounter, logger );

    // #4: ---------------- give the local mutex ----------------
    // from here the HP task gets xMutexIn then immeidately preempt LP task,
    mtxOpsStatus = xSemaphoreGive( xMutexLocal );
    TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, mtxOpsStatus, logger );

    // when LP task gets here, its priority should no longer be the same as the HP task.
    uxCurrentPriority = uxTaskPriorityGet( NULL );
    TEST_ASSERT_EQUAL_UINT32_LOGGER( (uxPriorityLPtsk+1), uxCurrentPriority, logger );
    // before this task, the LP task already added TEST_ADD_TWO to the shared counter.
    // here we simply recheck the value of the shared counter
    TEST_ASSERT_EQUAL_INT16_LOGGER( expectedCounterValue, sharedCounter, logger );

    // recover LP task's priority
    vTaskPrioritySet( NULL, uxPriorityLPtsk );
    // don't forget to delete the local mutex to avoid memory leak
    vSemaphoreDelete( xMutexLocal );
    xMutexLocal = NULL;
} //// end of vMtxKs1AccessMultiMutex2()






static void vMtxKs1LPtsk(void *pvParams)
{
    mtxTestParamStruct *mtxParams = (mtxTestParamStruct *) pvParams;
    for(;;)
    {
        vMtxKs1AccessMultiMutex1( mtxParams );
        vMtxKs1AccessMultiMutex2( mtxParams );
    }
} //// end of vMtxKs1LPtsk





static void vMtxKs1MPtsk(void *pvParams)
{
    mtxTestParamStruct *mtxParams = (mtxTestParamStruct *) pvParams;
    TestLogger_t          *logger = mtxParams->logger;
    UBaseType_t expectedCounterValue = TEST_ADD_TWO + TEST_ADD_FIVE ;

    for(;;)
    {
        vTaskSuspend(NULL);
        // before this task, the LP task already added a number to the shared counter.
        // here we simply recheck the value of the shared counter
        TEST_ASSERT_EQUAL_INT16_LOGGER( expectedCounterValue, sharedCounter, logger );
        sharedCounter += TEST_ADD_THREE;
    }
} //// end of vMtxKs1MPtsk





static void vMtxKs1HPtsk(void *pvParams)
{
    mtxTestParamStruct *mtxParams = (mtxTestParamStruct *) pvParams;
    SemaphoreHandle_t    xMutexIn = mtxParams->xMutex;
    TickType_t         xBlockTime = mtxParams->xBlockTime;
    TestLogger_t          *logger = mtxParams->logger;

    BaseType_t mtxOpsStatus;

    for(;;)
    {
        vTaskSuspend(NULL);
        // the HP task should wait on xSemaphoreTake() until LP task gives the shared mutex
        // , then HP task simply increases the shared counter, loop back & suspend itself again.
        mtxOpsStatus = xSemaphoreTake( xMutexIn, xBlockTime );

        if (mtxOpsStatus == pdPASS)
        {
            // before this task, the LP task already added a number to the shared counter.
            // here we simply recheck the value of the shared counter
            TEST_ASSERT_EQUAL_INT16_LOGGER( TEST_ADD_TWO, sharedCounter, logger );
            sharedCounter += TEST_ADD_FIVE;
            mtxOpsStatus   = xSemaphoreGive( xMutexIn );
            TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, mtxOpsStatus, logger );
        }
    }
} //// end of vMtxKs1HPtsk



void vStartMutexTestCase1( UBaseType_t uxPriority )
{
    void (*pvTaskFuncs[NUM_OF_TASKS])(void *) = { vMtxKs1LPtsk, vMtxKs1MPtsk, vMtxKs1HPtsk};
    const portCHAR      pcTaskName[NUM_OF_TASKS][16]   = { "MtxKs1LPtsk", "MtxKs1MPtsk", "MtxKs1HPtsk"};
    UBaseType_t         uxTaskPriorities[NUM_OF_TASKS] = { (uxPriority| portPRIVILEGE_BIT), 
                                                           ((uxPriority+2)| portPRIVILEGE_BIT), 
                                                           ((uxPriority+3)| portPRIVILEGE_BIT)  }; 
    TaskHandle_t       *xTaskHandlers[NUM_OF_TASKS] = { NULL, &xMtxKs1MPtsk_tcb, &xMtxKs1HPtsk_tcb};
    StackType_t        *stackMemSpace[NUM_OF_TASKS] ;
    BaseType_t          xState;
    unsigned portSHORT  idx;
    unsigned portSHORT  jdx;

    sharedCounter = 0;
    // we only create 2 structure variables for high-priority task and low-priority task
    // , no need to pass parameters to the medium-priority task
    mtxTestParamStruct *mtxParams[NUM_OF_TASKS];
    mtxParams[0] = (mtxTestParamStruct *) pvPortMalloc( sizeof(mtxTestParamStruct) );
    mtxParams[1] = (mtxTestParamStruct *) pvPortMalloc( sizeof(mtxTestParamStruct) );
    for (idx=0; idx<NUM_OF_TASKS; idx++) {
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc( sizeof(StackType_t) * intgSTACK_SIZE );
    }
    mtxParams[2] = (mtxTestParamStruct *) pvPortMalloc( sizeof(mtxTestParamStruct) );
    configASSERT( mtxParams[0] );
    configASSERT( mtxParams[1] );
    configASSERT( mtxParams[2] );
    mtxParams[0]->xBlockTime = 0x0;
    mtxParams[1]->xBlockTime = 0x0;
    mtxParams[2]->xBlockTime = portMAX_DELAY;
    mtxParams[0]->logger     = xRegisterNewTestLogger( __FILE__ , "mutex test (case 1) -- low-priority task");
    mtxParams[1]->logger     = xRegisterNewTestLogger( __FILE__ , "mutex test (case 1) -- medium-priority task");
    mtxParams[2]->logger     = xRegisterNewTestLogger( __FILE__ , "mutex test (case 1) -- high-priority task");
    mtxParams[0]->xMutex = xSemaphoreCreateMutex();
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
} //// end of vStartMutexTestCase1()

