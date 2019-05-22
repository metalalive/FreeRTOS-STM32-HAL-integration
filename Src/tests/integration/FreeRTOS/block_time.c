#include "tests/integration/FreeRTOS/block_time.h"

#define NUM_OF_TASKS        2 
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define intgSTACK_SIZE       ( ( unsigned portSHORT ) 0x3e )
#define  SHARED_Q_NO_BLOCK_TIME  0
#define  SHARED_Q_LENGTH         0xa
#define  SECOND_TASK_RDY_FLAG    0x55

static volatile QueueHandle_t xSharedQueue;
static volatile TaskHandle_t  xBlkTimSecondary ;
// margin is required since the estimated delay time is not 100% accurate
static const TickType_t  xAllowableMargin = 7;
// possible values in the shared queue items, only for this integration test
static const BaseType_t  possibleQItemValue[SHARED_Q_LENGTH * 2] = {-81 ,13, -3, -5, 63, -97, 17, -29, 41, -53, 101, -1003, 257, 31, 26, -16};
// set to SECOND_TASK_RDY_FLAG whenever task 2 vBlockTimeSecondary() is ready, this flag
// is internally used in this integration test, otherwise this variable should be zero
static volatile portCHAR ucSecondaryTaskReadyFlag = 0;




static void vDelayFunctionTest(TestLogger_t *logger)
{
    UBaseType_t uxPriority = uxTaskPriorityGet(NULL);
    const TickType_t  uxBlockTimeBase = 75;
    TickType_t  xStartTime = 0;
    TickType_t  xEndTime   = 0;
    TickType_t  xAllowableDelay = 0;
    TickType_t  xActualDelay = 0;
    TickType_t  xLastUnblockTimeTick = 0;
    unsigned portSHORT   uxIteration = 5;
    unsigned portSHORT   idx = 0;

    // Since we will run all integration tests concurrently, with application tasks of different priority,
    // In this function, in order to get precise estimated delay time, we'll have to 
    // temporarily increase current priority of task, but not as high as the max priority
    // for FreeRTOs system call (the configMAX_PRIORITIES) ,
    // so we can prevent this task from being preempted by other higher-priority application tasks,
    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);

    // ------ crude check to estimated delayed time of vTaskDelay() ------
    xStartTime = xTaskGetTickCount();
    vTaskDelay( uxBlockTimeBase );
    xEndTime   = xTaskGetTickCount();

    GET_TICKS_INTERVAL(xStartTime, xEndTime, xActualDelay);
    xAllowableDelay = uxBlockTimeBase + xAllowableMargin;

    TEST_ASSERT_GREATER_OR_EQUAL_UINT32_LOGGER( uxBlockTimeBase, xActualDelay, logger );
    TEST_ASSERT_LESS_THAN_UINT32_LOGGER(    xAllowableDelay, xActualDelay, logger );

    // ------ crude check to estimated delayed time of vTaskDelayUntil() ------
    xStartTime = xTaskGetTickCount();
    xLastUnblockTimeTick = xStartTime;

    for(idx=0; idx<uxIteration; idx++)
    {
        // vTaskDelayUntil() internally appends uxBlockTimeBase to xLastUnblockTimeTick, which means the current
        // task will get ready again after the amount of specified delay time (uxBlockTimeBase) passes.
        vTaskDelayUntil( &xLastUnblockTimeTick, uxBlockTimeBase );
        xEndTime = xTaskGetTickCount();

        GET_TICKS_INTERVAL(xStartTime, xEndTime, xActualDelay);
        // xStartTime is not changed in this loop, we adjust maximum allowable delay time.
        xAllowableDelay = (idx + 1) * uxBlockTimeBase + xAllowableMargin;
        TEST_ASSERT_LESS_THAN_UINT32_LOGGER( xAllowableDelay, xActualDelay, logger );
    }

    // recover the priority of current task
    vTaskPrioritySet(NULL, uxPriority);
} //// end of vDelayFunctionTest()




static void vDlyTimeQueueRecvTest (TestLogger_t *logger)
{
    const TickType_t  uxBlockTimeBase = 10;
    TickType_t  xStartTime = 0;
    TickType_t  xEndTime   = 0;
    TickType_t  xExpectedDelay  = 0;
    TickType_t  xActualDelay = 0;
    BaseType_t  QoperationStatus = pdPASS;
    BaseType_t  qItem = 0x0;
    unsigned portSHORT   uxIteration = 5;
    unsigned portSHORT   idx = 0;

    for(idx=0; idx<uxIteration; idx++)
    {
        QoperationStatus = pdPASS;
        xExpectedDelay = uxBlockTimeBase << idx;
        // when current task runs at here, the shared queue is empty, we call xQueueReceive()
        // only for testing how accurate the blocking time is.
        xStartTime = xTaskGetTickCount();
        QoperationStatus = xQueueReceive( xSharedQueue, &qItem, xExpectedDelay );
        xEndTime   = xTaskGetTickCount();

        GET_TICKS_INTERVAL(xStartTime, xEndTime, xActualDelay);
        // the shared queue must be empty therefore it should return errQUEUE_EMPTY
        TEST_ASSERT_EQUAL_UINT32_LOGGER( errQUEUE_EMPTY,QoperationStatus, logger );
        // check the estimated blocking time
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32_LOGGER( xExpectedDelay, xActualDelay, logger );
        TEST_ASSERT_LESS_THAN_UINT32_LOGGER(   (xExpectedDelay + xAllowableMargin), xActualDelay, logger );
    }
} //// end of vDlyTimeQueueRecvTest()




static void vDlyTimeQueueSendTest (TestLogger_t *logger)
{
    const TickType_t  uxBlockTimeBase = 10;
    TickType_t  xStartTime = 0;
    TickType_t  xEndTime   = 0;
    TickType_t  xExpectedDelay = 0;
    TickType_t  xActualDelay = 0;
    BaseType_t  QoperationStatus = pdPASS;
    BaseType_t  qItem = 0x0;
    unsigned portSHORT   uxIteration = 5;
    unsigned portSHORT   idx = 0;

    // fill the shared queue before starting this part of test
    for(idx=0; idx<SHARED_Q_LENGTH ; idx++)
    {
        qItem = possibleQItemValue[idx];
        QoperationStatus = errQUEUE_FULL;
        QoperationStatus = xQueueSend( xSharedQueue, &qItem, 0);
        TEST_ASSERT_EQUAL_UINT32_LOGGER( pdPASS, QoperationStatus, logger );
    }

    qItem = possibleQItemValue[8];

    for(idx=0; idx<uxIteration; idx++)
    {
        QoperationStatus = pdPASS;
        xExpectedDelay = uxBlockTimeBase << idx;
        // when current task runs at here, the shared queue is already full, we call xQueueSend()
        // only for testing how accurate the blocking time is.
        xStartTime = xTaskGetTickCount();
        QoperationStatus = xQueueSend( xSharedQueue, &qItem, xExpectedDelay );
        xEndTime   = xTaskGetTickCount();
        GET_TICKS_INTERVAL(xStartTime, xEndTime, xActualDelay);
        // the shared queue must be full therefore it should return errQUEUE_FULL
        TEST_ASSERT_EQUAL_UINT32_LOGGER( errQUEUE_FULL, QoperationStatus, logger );
        // check the estimated blocking time
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32_LOGGER( xExpectedDelay, xActualDelay, logger );
        TEST_ASSERT_LESS_THAN_UINT32_LOGGER(   (xExpectedDelay + xAllowableMargin), xActualDelay, logger );
    }
} //// end of vDlyTimeQueueSendTest()




static void vBlockTimePrimary(void *pvParams)
{
    TestLogger_t *logger = (TestLogger_t *) pvParams;
    const TickType_t  uxBlockTimeBase = 25;
    BaseType_t  QoperationStatus = pdPASS;
    BaseType_t  qItem = 0x0;
    UBaseType_t SecondTaskOriginPriority  = 0x0;
    UBaseType_t PrimaryTaskOriginPriority = 0x0;
    unsigned portSHORT   uxIteration = 3;
    unsigned portSHORT   idx = 0;

    PrimaryTaskOriginPriority = uxTaskPriorityGet( NULL );
    SecondTaskOriginPriority  = uxTaskPriorityGet( xBlkTimSecondary );
    vTaskDelay( 0x1 ); // it's optional to add a bit of delay to this test

    for(;;)
    {
        // ----- part 0 : basic delay function tests on a single task -----
        // basic vTaskDelay() and vTaskDelayUntil() tests
        vDelayFunctionTest( logger );

        // ----- part 1 : block time test with xQueueReceive() on a single task  -----
        vDlyTimeQueueRecvTest( logger );
        
        // ----- part 2 : block time test with xQueueSend() on a single task  -----
        vDlyTimeQueueSendTest( logger );

        // ----- part 3 : -----
        // this task cooperates with task 2 vBlockTimeSecondary() on this part of the test
        // this task accesses the shared queue in a way that makes the task 2 always fail to
        // send item to the shared queue, therefore leads to timeout in task 2 because 
        // certain amount of blocking time passed in task 2,
        // then we check how accurate the blocking time is.
        ucSecondaryTaskReadyFlag = 0x0;
        // wake task 2 but not switch to it yet because this task has higher priority.
        vTaskResume( xBlkTimSecondary );

        // give some CPU time to task 2, then the ready flag (ucSecondaryTaskReadyFlag)
        // should be set right before the task 2 gets blocked due to xQueueSend()
        // (send item to the full shared queue)
        while(ucSecondaryTaskReadyFlag != SECOND_TASK_RDY_FLAG)
        {
            vTaskDelay( uxBlockTimeBase );
        }
        // Delay this task once more to prevent a situation that task 2 already set the ready
        // flag but wasn't blocked by xQueueSend()
        vTaskDelay( uxBlockTimeBase );
        ucSecondaryTaskReadyFlag = 0x0;
 
        for(idx=0; idx<uxIteration; idx++) 
        {
            // read item from the front of the shared queue then immediately write it to the back,
            // the queue operations below should be done & return immediately without any delay,
            QoperationStatus = xQueueReceive( xSharedQueue, &qItem, SHARED_Q_NO_BLOCK_TIME );
            TEST_ASSERT_EQUAL_UINT32_LOGGER( pdPASS, QoperationStatus, logger );
            QoperationStatus = xQueueSend(    xSharedQueue, &qItem, SHARED_Q_NO_BLOCK_TIME );
            TEST_ASSERT_EQUAL_UINT32_LOGGER( pdPASS, QoperationStatus, logger );
            // increase the priority of task 2, at this point task 2 should NOT take CPU control because it 
            // is supposed to be blocked at xQueueSend()
            vTaskPrioritySet( xBlkTimSecondary, PrimaryTaskOriginPriority + 1 );
            // check if task 2 was woken from xQueueSend()
            TEST_ASSERT_EQUAL_UINT32_LOGGER( 0x0, ucSecondaryTaskReadyFlag, logger );
            vTaskPrioritySet( xBlkTimSecondary, SecondTaskOriginPriority);
        }

        // task 2 will be woken due to the timeout of xQueueSend(), finally sets the ready flag to 
        // SECOND_TASK_RDY_FLAG to finish this part of the test
        while(ucSecondaryTaskReadyFlag != SECOND_TASK_RDY_FLAG)
        {
            vTaskDelay( uxBlockTimeBase >> 1 );
        }
        vTaskDelay( uxBlockTimeBase );

 
        // ----- part 4 : -----
        // similar to part 3, except task 2 will always fail to receive any item
        // from the shared queue. For explanation of each line of code please see part 3 above.

        // empty the shared queue before starting this part of test
        for(idx=0; idx<SHARED_Q_LENGTH ; idx++)
        {
            QoperationStatus = xQueueReceive( xSharedQueue, &qItem, SHARED_Q_NO_BLOCK_TIME );
            TEST_ASSERT_EQUAL_UINT32_LOGGER( pdPASS, QoperationStatus, logger );
        }

        ucSecondaryTaskReadyFlag = 0x0;
        vTaskResume( xBlkTimSecondary );
        while(ucSecondaryTaskReadyFlag != SECOND_TASK_RDY_FLAG)
        {
            vTaskDelay( uxBlockTimeBase );
        }
        vTaskDelay( uxBlockTimeBase );
        ucSecondaryTaskReadyFlag = 0x0;

        for(idx=0; idx<uxIteration; idx++) 
        {
            qItem = possibleQItemValue[idx];
            QoperationStatus = xQueueSend(    xSharedQueue, &qItem, SHARED_Q_NO_BLOCK_TIME );
            TEST_ASSERT_EQUAL_UINT32_LOGGER( pdPASS, QoperationStatus, logger );
            QoperationStatus = xQueueReceive( xSharedQueue, &qItem, SHARED_Q_NO_BLOCK_TIME );
            TEST_ASSERT_EQUAL_UINT32_LOGGER( pdPASS, QoperationStatus, logger );
            vTaskPrioritySet( xBlkTimSecondary, PrimaryTaskOriginPriority + 1);
            TEST_ASSERT_EQUAL_UINT32_LOGGER( 0x0, ucSecondaryTaskReadyFlag, logger );
            ////TEST_COUNT_ERROR_NE( ucSecondaryTaskReadyFlag, 0x0, error_flag_ptr ); 
            vTaskPrioritySet( xBlkTimSecondary, SecondTaskOriginPriority);
        }

        while(ucSecondaryTaskReadyFlag != SECOND_TASK_RDY_FLAG)
        {
            vTaskDelay( uxBlockTimeBase >> 1 );
        }
        vTaskDelay( uxBlockTimeBase );

    } //// end of the outer infinite loop
} //// end of vBlockTimePrimary()




static void vBlockTimeSecondary(void *pvParams)
{
    TestLogger_t *logger = (TestLogger_t *) pvParams;
    const TickType_t  uxBlockTimeBase = 140;
    TickType_t  xActualDelay = 0;
    BaseType_t  QoperationStatus = pdPASS;
    BaseType_t  qItem = 0x0;
    TickType_t  xStartTime = 0;
    TickType_t  xEndTime   = 0;

    for(;;)
    {
        // ----- part 0, 1, 2: this task doesn't do anything -----
        vTaskSuspend( NULL );

        // ----- part 3 : -----
        // see description in part 3 of task 1, the vBlockTimePrimary()
        xStartTime = xTaskGetTickCount();
        // set the ready flag, then call xQueueSend(), where the shared queue is already full
        // then this task get blocked. 
        // Note that the blocking time here should be long enough for task 1
        ucSecondaryTaskReadyFlag = SECOND_TASK_RDY_FLAG;
        QoperationStatus = xQueueSend( xSharedQueue, &qItem, uxBlockTimeBase );
        xEndTime = xTaskGetTickCount();
        // estimate blocking time when this task is woken from the xQueueSend() above.
        GET_TICKS_INTERVAL(xStartTime, xEndTime, xActualDelay);
        // the shared queue must be full therefore xQueueSend() should return errQUEUE_FULL 
        TEST_ASSERT_EQUAL_UINT32_LOGGER( errQUEUE_FULL, QoperationStatus, logger );
        // check if the estimated blocking time is out of range
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32_LOGGER( uxBlockTimeBase, xActualDelay, logger );
        TEST_ASSERT_LESS_THAN_UINT32_LOGGER(   (uxBlockTimeBase + xAllowableMargin), xActualDelay, logger );

        // set the ready flag again 
        ucSecondaryTaskReadyFlag = SECOND_TASK_RDY_FLAG;
        vTaskSuspend( NULL );
        
        // ----- part 4 : -----
        // see description in part 4 of task 1, the vBlockTimePrimary()
        // For explanation of each line of code please see part 3 above.
        xStartTime = xTaskGetTickCount();
        ucSecondaryTaskReadyFlag = SECOND_TASK_RDY_FLAG;
        QoperationStatus = xQueueReceive( xSharedQueue, &qItem, uxBlockTimeBase );
        xEndTime = xTaskGetTickCount();
        GET_TICKS_INTERVAL(xStartTime, xEndTime, xActualDelay);
        TEST_ASSERT_EQUAL_UINT32_LOGGER( errQUEUE_EMPTY, QoperationStatus, logger );
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32_LOGGER( uxBlockTimeBase, xActualDelay, logger );
        TEST_ASSERT_LESS_THAN_UINT32_LOGGER(       (uxBlockTimeBase + xAllowableMargin), xActualDelay, logger );

        // set the ready flag again to make task 1 iterate whole test procedures again.
        ucSecondaryTaskReadyFlag = SECOND_TASK_RDY_FLAG;
    } //// end of outer infinite loop
} //// end of vBlockTimeSecondary()




void vStartBlockTimeTasks( UBaseType_t uxPriority )
{
    const UBaseType_t    taskPriority[NUM_OF_TASKS] = { 
                                                         (UBaseType_t) ((uxPriority+1) | portPRIVILEGE_BIT),
                                                         (UBaseType_t) (uxPriority | portPRIVILEGE_BIT),
                                                      };
    StackType_t     *stackMemSpace[NUM_OF_TASKS] ;
    TestLogger_t    *tlogger[2];
    BaseType_t       xState; 
    portSHORT        idx;

    xSharedQueue = xQueueCreate( SHARED_Q_LENGTH, sizeof(BaseType_t) );
    configASSERT( xSharedQueue );
    // vQueueAddToRegistry() adds the queue to the queue registry, which is provided as for kernel aware
    // debuggers, 
    #if (configQUEUE_REGISTRY_SIZE > 0)
        vQueueAddToRegistry( xSharedQueue, "Block_Time_Queue");
    #endif //// end of configQUEUE_REGISTRY_SIZE

    tlogger[0] = xRegisterNewTestLogger( __FILE__ , "blocking time test -- primary task" );
    tlogger[1] = xRegisterNewTestLogger( __FILE__ , "blocking time test -- secondary task" );
    // [TODO] recheck address alignment of allocated stack memory.
    // allocate stack memory for each task
    stackMemSpace[0] = (StackType_t *) pvPortMalloc(sizeof(StackType_t) * intgSTACK_SIZE );
    stackMemSpace[1] = (StackType_t *) pvPortMalloc(sizeof(StackType_t) * intgSTACK_SIZE );
    // internally create structure to collect parameters feeding to task creating function
    TaskParameters_t tsk1params = {
        vBlockTimePrimary, "BlkTimPrim", intgSTACK_SIZE, (void *)tlogger[0],
        taskPriority[0], stackMemSpace[0], 
        // leave MPU regions uninitialized
    }; 
    TaskParameters_t tsk2params = {
        vBlockTimeSecondary, "BlkTimSec", intgSTACK_SIZE, (void *)tlogger[1],
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
    xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tsk2params, &xBlkTimSecondary );
    configASSERT( xState == pdPASS );
} //// end of vStartBlockTimeTasks

