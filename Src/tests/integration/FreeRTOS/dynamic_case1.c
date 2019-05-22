#include "tests/integration/FreeRTOS/dynamic_case1.h"

#define NUM_OF_TASKS        3 
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define intgSTACK_SIZE       ( ( unsigned portSHORT ) 0x3e )


// in some cases, the following TCB_t pointers would be optimized out
// by GCC toolchain, so it is required to add volatile ahead of these pointers.
static volatile TaskHandle_t pxDynPrContnIncr;
static volatile TaskHandle_t pxDynPrBatchIncr;
// the shared counter among the tasks in this test
static volatile UBaseType_t   sharedCounter;
static const portSHORT maxCounterValue = 500;



static void vDynPrContinuousIncrementHandler(void *pvParams)
{
    UBaseType_t uxMyOriginPriority = uxTaskPriorityGet(NULL);
    for(;;)
    {
        // temporarily increase its priority number, add shared counter by 1 unit,
        // then recover its priority back
        vTaskPrioritySet(NULL, uxMyOriginPriority + 1);
        sharedCounter++;
        vTaskPrioritySet(NULL, uxMyOriginPriority);
    }//// end of outer infinite loop
} //// end of vDynPrContinuousIncrementHandler


static void vDynPrBatchIncrementHandler(void *pvParams)
{
    portSHORT idx = 0;
    for(;;)
    {
        vTaskSuspend(NULL);
        // task 2 is resumed from here, since we don't modify the priority
        // of this task, this task can count up sharedCounter without being preempted
        for(idx=0; idx<maxCounterValue; idx++)
        {
            sharedCounter++ ;
        }
    }//// end of outer infinite loop
} //// end of vDynPrBatchIncrementHandler



static void vDynPrControlTasksHandler(void *pvParams)
{
    int32_t expectedVal  = 0; 
    int32_t actualVal    = 0;
    UBaseType_t  lastSharedCounter = 0;
    portSHORT    idx = 0;
    const portSHORT maxIterationPart1 = 7;
    TestLogger_t *logger = (TestLogger_t *) pvParams;
    
    for(;;)
    {
        // ---- the first part ----
        // this task yields CPU control then task 1 (vDynPrContinuousIncrementHandler)
        // takes turn to execute, since the higher-priority task 2 is still in suspended state.
        sharedCounter = 0;

        // the loop checks if sharedCounter is increased by task 1.
        for(idx=0; idx<maxIterationPart1; idx++)
        {
            // there will be data corruption if context switch happens before underlying hardware 
            // writes the copied data of sharedCounter to lastSharedCounter, 
            // It is required to suspend task 1 for data synchronization,
            // However in the first part of this integration test, 
            // we are only concerned about whether sharedCounter is increased by task 1 in each iteration,
            // it is NOT necessary to check data consistancy at here (the shared counter sharedCounter)
            // between this task and task 1, 
            // so it is optional to suspend task 1 only for this test.
            vTaskSuspend( pxDynPrContnIncr );
            lastSharedCounter = sharedCounter;
            vTaskResume( pxDynPrContnIncr );
            // this task gives up CPU control for a while, 
            // it's time for task 1 to count up sharedCounter
            vTaskDelay((TickType_t)50);
            // when CPU gets here, sharedCounter should be greater than 0.
            // stop context switch by temporarily disabling scheduler,
            // SysTick interrrupt still makes CPU to its ISR, 
            // but RTOS scheduler just doesn't work for a short while
            vTaskSuspendAll();
            // Check the value of sharedCounter
            TEST_ASSERT_GREATER_THAN_UINT32_LOGGER( lastSharedCounter, sharedCounter, logger );
            xTaskResumeAll();
        }

        // ---- the second part ----
        // suspend task 1 for simplicity.
        vTaskSuspend( pxDynPrContnIncr );
        sharedCounter = 0;
        // resume task 2, then kernel immediately performs context switch to task 2.
        vTaskResume( pxDynPrBatchIncr );
        // when CPU gets here, task 2 should have already added maxCounterValue to sharedCounter.
        TEST_ASSERT_EQUAL_UINT32_LOGGER( maxCounterValue, sharedCounter, logger );
        // resume task 1, then go back to the first part above again.
        vTaskResume( pxDynPrContnIncr );
    } //// end of outer infinite loop
} //// end of vDynPrControlTasksHandler


void vStartDynamicPriorityCase1( UBaseType_t uxPriority )
{
    const char           tasknames[NUM_OF_TASKS][16] = { "DynPrContnIncr","DynPrBatchIncr","DynPrCtrlTasks", };
    const TaskFunction_t taskFuncs[NUM_OF_TASKS] = { vDynPrContinuousIncrementHandler,
                                                     vDynPrBatchIncrementHandler,
                                                     vDynPrControlTasksHandler,
                                                  };
    const UBaseType_t    taskPriority[NUM_OF_TASKS] = { 
                                                         (UBaseType_t) (uxPriority | portPRIVILEGE_BIT),
                                                         (UBaseType_t) ((uxPriority+1) | portPRIVILEGE_BIT),
                                                         (UBaseType_t) (uxPriority | portPRIVILEGE_BIT),
                                                      };
    StackType_t     *stackMemSpace[NUM_OF_TASKS] ;
    TestLogger_t    *tloggers[NUM_OF_TASKS];
    TaskHandle_t     tskHandlers[NUM_OF_TASKS];
    BaseType_t    xState; 
    portSHORT     idx;
    portSHORT     jdx;

    tloggers[0] = NULL;
    tloggers[1] = NULL;
    tloggers[2] = xRegisterNewTestLogger( __FILE__ , "dynamic priority control task (case 1)" );
    for(idx=0; idx<NUM_OF_TASKS; idx++) {
        // allocate stack memory for each task
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc(sizeof(StackType_t) * intgSTACK_SIZE );
    }

    for(idx=0; idx<NUM_OF_TASKS; idx++) 
    {
        TaskParameters_t tskparams = {
            taskFuncs[idx], &tasknames[idx], intgSTACK_SIZE , 
            (void *) tloggers[idx], taskPriority[idx], stackMemSpace[idx], 
            // leave MPU regions uninitialized
        }; 
        // default value to xRegions 
        for(jdx=0; jdx<portNUM_CONFIGURABLE_REGIONS; jdx++)
        {
            tskparams.xRegions[jdx].pvBaseAddress   = NULL;
            tskparams.xRegions[jdx].ulLengthInBytes = 0;
            tskparams.xRegions[jdx].ulParameters    = 0;
        }
        xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tskparams, &tskHandlers[idx] );
        configASSERT( xState == pdPASS );
    } //// end of loop

    pxDynPrContnIncr = tskHandlers[0];
    pxDynPrBatchIncr = tskHandlers[1];

} // vStartDynamicPriorityCase1


