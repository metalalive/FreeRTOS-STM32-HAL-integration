#include "tests/integration/FreeRTOS/semphr_cnt.h"

#define  NUM_OF_TASKS            2
#define  SMPH_CNT_MAX_LENGTH     5
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define  intgSTACK_SIZE          (( unsigned portSHORT ) 0x3e)

// the data structure below is used only for passing parameters to the tasks in this tests
typedef struct {
    // the queue used by the tasks
    SemaphoreHandle_t     xSemphr; 
    // the blocking time can be applied to Queue send/receive operations, or task delay functions
    TickType_t            xBlockTime;
    // for logging assertion failure
    TestLogger_t         *logger; 
} smphrParamStruct;



static void vDecrSemphrCountToZero(smphrParamStruct *lclsemParams, UBaseType_t  *currentCountValue)
{
    SemaphoreHandle_t     xSemphr     = lclsemParams->xSemphr ; 
    TickType_t            xBlockTime  = lclsemParams->xBlockTime ;
    TestLogger_t         *logger      = lclsemParams->logger;

    BaseType_t            semOpsStatus;
    portSHORT             idx;

    // intially, the semaphore shouldn't be able to be given.
    semOpsStatus = xSemaphoreGive( xSemphr );
    TEST_ASSERT_NOT_EQUAL_LOGGER( pdPASS, semOpsStatus, logger );
    // continuously takes the semaphore until the count value reaches zero
    for(idx=0; idx<SMPH_CNT_MAX_LENGTH; idx++)
    {
        TEST_ASSERT_EQUAL_UINT32_LOGGER( (SMPH_CNT_MAX_LENGTH - idx), uxSemaphoreGetCount(xSemphr), logger );
        semOpsStatus = xSemaphoreTake( xSemphr, xBlockTime );
        TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, semOpsStatus, logger );
    }
    *currentCountValue = uxSemaphoreGetCount( xSemphr );
} //// end of vDecrSemphrCountToZero()




static void vIncrSemphrCountToMax(smphrParamStruct *lclsemParams, UBaseType_t  *currentCountValue)
{
    SemaphoreHandle_t     xSemphr     = lclsemParams->xSemphr ; 
    TickType_t            xBlockTime  = lclsemParams->xBlockTime ;
    TestLogger_t         *logger      = lclsemParams->logger;

    BaseType_t            semOpsStatus;
    portSHORT             idx;

    // intially, the semaphore shouldn't be able to be taken.
    semOpsStatus = xSemaphoreTake( xSemphr, xBlockTime );
    TEST_ASSERT_NOT_EQUAL_LOGGER( pdPASS, semOpsStatus, logger );
    // continuously gives the semaphore until the count value reaches its defined maximum
    for(idx=0; idx<SMPH_CNT_MAX_LENGTH; idx++)
    {
        TEST_ASSERT_EQUAL_UINT32_LOGGER( idx, uxSemaphoreGetCount(xSemphr), logger );
        semOpsStatus = xSemaphoreGive( xSemphr );
        TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, semOpsStatus, logger );
    }
    *currentCountValue = uxSemaphoreGetCount( xSemphr );
} //// end of vIncrSemphrCountToMax




static void vCntSmphrTestTsk(void *pvParams)
{
    smphrParamStruct *lclsemParams = (smphrParamStruct *) pvParams;
    UBaseType_t  currentCountValue;
    // check initial value of the counting semaphore 
    currentCountValue = uxSemaphoreGetCount( lclsemParams->xSemphr );
    if(currentCountValue == SMPH_CNT_MAX_LENGTH) {
        vDecrSemphrCountToZero( lclsemParams, &currentCountValue );
    }

    for(;;)
    {
        vIncrSemphrCountToMax(  lclsemParams, &currentCountValue );
        vDecrSemphrCountToZero( lclsemParams, &currentCountValue );
        taskYIELD();
    }
} //// end of vCntSmphrTestTsk()




void vStartCountSemphrTest( UBaseType_t uxPriority )
{
    // collect parameters to the following structure that can be accessed by tasks
    volatile smphrParamStruct *semParams[NUM_OF_TASKS];
    StackType_t        *stackMemSpace[NUM_OF_TASKS] ;
    BaseType_t          xState; 
    unsigned portSHORT  idx;

    for (idx=0; idx<NUM_OF_TASKS; idx++) {
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc( sizeof(StackType_t) * intgSTACK_SIZE );
    }
    semParams[0] = (smphrParamStruct *) pvPortMalloc( sizeof(smphrParamStruct) );
    semParams[1] = (smphrParamStruct *) pvPortMalloc( sizeof(smphrParamStruct) );
    configASSERT( semParams[0] );
    configASSERT( semParams[1] );
    semParams[0]->xSemphr = xSemaphoreCreateCounting( SMPH_CNT_MAX_LENGTH, 0 );
    semParams[1]->xSemphr = xSemaphoreCreateCounting( SMPH_CNT_MAX_LENGTH, SMPH_CNT_MAX_LENGTH );
    configASSERT( semParams[0]->xSemphr );
    configASSERT( semParams[1]->xSemphr );
    semParams[0]->xBlockTime = 0;
    semParams[1]->xBlockTime = 0;
    semParams[0]->logger     = xRegisterNewTestLogger( __FILE__ , "Counting semaphore test -- task 1");
    semParams[1]->logger     = xRegisterNewTestLogger( __FILE__ , "Counting semaphore test -- task 2");
    configASSERT( semParams[0]->logger );
    configASSERT( semParams[1]->logger );

    TaskParameters_t tsk1params = {
        vCntSmphrTestTsk, "CSemTstTsk1", intgSTACK_SIZE, (void *)semParams[0],
        (uxPriority | portPRIVILEGE_BIT), stackMemSpace[0],
        // leave MPU regions uninitialized
    };
    TaskParameters_t tsk2params = {
        vCntSmphrTestTsk, "CSemTstTsk2", intgSTACK_SIZE, (void *)semParams[1],
        (uxPriority | portPRIVILEGE_BIT), stackMemSpace[1],
        // leave MPU regions uninitialized
    };
    // default value to unused MPU regions 
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
    xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tsk2params, NULL );
    configASSERT( xState == pdPASS );

} //// end of vStartCountSemphrTest

