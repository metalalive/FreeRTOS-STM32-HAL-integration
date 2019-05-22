#include "tests/integration/FreeRTOS/semphr_bin_case1.h"

#define  NUM_OF_TASKS  4
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define  intgSTACK_SIZE      (( unsigned portSHORT ) 0x3e)


// the data structure below is used only for passing parameters to the tasks in this tests
typedef struct {
    // the queue used by the tasks
    SemaphoreHandle_t     xSemphr; 
    // the blocking time can be applied to Queue send/receive operations, or task delay functions
    TickType_t            xBlockTime;
    // shared variables used in this test
    volatile UBaseType_t *pulSharedVariable;
    // for logging assertion failure
    TestLogger_t         *logger; 
} smphrParamStruct;



static void vBinSmphrKs1Giver(void *pvParams)
{
    smphrParamStruct *semParams = (smphrParamStruct *) pvParams;
    SemaphoreHandle_t     xSemphr            = semParams->xSemphr ; 
    TickType_t            xBlockTime         = semParams->xBlockTime ;
    TestLogger_t         *logger             = semParams->logger;
    volatile UBaseType_t *pulSharedVariable  = NULL ;

    taskENTER_CRITICAL();
    pulSharedVariable  = semParams->pulSharedVariable ;
    taskEXIT_CRITICAL();
    UBaseType_t  pulSharedVariableBackup = *pulSharedVariable;
    UBaseType_t  idx  =  0;

    for (;;)
    {
        // following lines of code ensure that the taker gets blocked on xSemaphoreTake()
        while ((*pulSharedVariable) != 0) {
            taskYIELD(); 
        }
        taskYIELD();
        taskYIELD();
        taskYIELD();
        
        // counts up the shared variable
        for (idx=1; idx<=pulSharedVariableBackup; idx++)
        {
             taskENTER_CRITICAL();
            *pulSharedVariable += 1;
             taskEXIT_CRITICAL();
            TEST_ASSERT_EQUAL_UINT32_LOGGER( idx, (*pulSharedVariable), logger );
        }
        // give the semaphore, let the other task take it .
        xSemaphoreGive( xSemphr );
    }
} //// end of vBinSmphrKs1Giver




static void vBinSmphrKs1Taker(void *pvParams)
{
    smphrParamStruct *semParams = (smphrParamStruct *) pvParams;
    SemaphoreHandle_t     xSemphr            = semParams->xSemphr ; 
    TickType_t            xBlockTime         = semParams->xBlockTime ;
    TestLogger_t         *logger             = semParams->logger;
    volatile UBaseType_t *pulSharedVariable  = NULL ;

    taskENTER_CRITICAL();
    pulSharedVariable  = semParams->pulSharedVariable ;
    taskEXIT_CRITICAL();
    UBaseType_t  pulSharedVariableBackup = *pulSharedVariable;
    UBaseType_t  pulCurrentPossibleValue = 0;
    UBaseType_t  idx  =  0;
    BaseType_t   semOpsStatus;

    for (;;)
    {
        taskENTER_CRITICAL();
        *pulSharedVariable = 0;
        taskEXIT_CRITICAL();
        // loop at here until this task successfully takes the semaphore
        while (1) {
            semOpsStatus = xSemaphoreTake( xSemphr, xBlockTime );
            if(semOpsStatus == pdPASS) {
                break;
            }
            else {
                taskYIELD();
            }
        }
        // check value of shared variable
        pulCurrentPossibleValue = pulSharedVariableBackup;
        TEST_ASSERT_EQUAL_UINT32_LOGGER( pulCurrentPossibleValue, (*pulSharedVariable), logger );
        // counts up the shared variable then the the value again
        for (idx=1; idx<=pulSharedVariableBackup; idx++)
        {
             taskENTER_CRITICAL();
            *pulSharedVariable += 1;
             taskEXIT_CRITICAL();
        }
        taskYIELD();
        pulCurrentPossibleValue = pulSharedVariableBackup << 1;
        TEST_ASSERT_EQUAL_UINT32_LOGGER( pulCurrentPossibleValue, (*pulSharedVariable), logger );
        taskYIELD();
    }
} //// end of vBinSmphrKs1Taker()




void vStartBinSemphrCase1( UBaseType_t uxPriority )
{
    smphrParamStruct   *semParams[ NUM_OF_TASKS / 2 ];
    const UBaseType_t   expected_init_shared_var[NUM_OF_TASKS / 2] = {13, 47};
    void   (*pvTaskFuncs[NUM_OF_TASKS])(void *) = {
                                                      vBinSmphrKs1Giver,  vBinSmphrKs1Taker,
                                                      vBinSmphrKs1Giver,  vBinSmphrKs1Taker,
                                                  };
    const portCHAR      pcTaskName[NUM_OF_TASKS][16] = {
                                                         "BSemKs1Giver1","BSemKs1Taker1",
                                                         "BSemKs1Giver2","BSemKs1Taker2",
                                                       };
    StackType_t        *stackMemSpace[NUM_OF_TASKS] ;
    BaseType_t          xState; 
    unsigned portSHORT  idx;
    unsigned portSHORT  jdx;

    for (idx=0; idx<NUM_OF_TASKS; idx++) {
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc( sizeof(StackType_t) * intgSTACK_SIZE );
    }
    // initialize parameters that will be used in this test
    semParams[0] = (smphrParamStruct *) pvPortMalloc( sizeof(smphrParamStruct) );
    semParams[1] = (smphrParamStruct *) pvPortMalloc( sizeof(smphrParamStruct) );
    configASSERT( semParams[0] );
    configASSERT( semParams[1] );
    semParams[0]->xSemphr = xSemaphoreCreateBinary();
    semParams[1]->xSemphr = xSemaphoreCreateBinary();
    configASSERT( semParams[0]->xSemphr );
    configASSERT( semParams[1]->xSemphr );
    semParams[0]->xBlockTime = 0;
    semParams[1]->xBlockTime = 100;
    semParams[0]->logger = xRegisterNewTestLogger( __FILE__ , "Binary semaphore test (case 1) -- scenario #1");
    semParams[1]->logger = xRegisterNewTestLogger( __FILE__ , "Binary semaphore test (case 1) -- scenario #2");
    configASSERT( semParams[0]->logger );
    configASSERT( semParams[1]->logger );
    semParams[0]->pulSharedVariable = (UBaseType_t *) pvPortMalloc( sizeof(UBaseType_t) );
    semParams[1]->pulSharedVariable = (UBaseType_t *) pvPortMalloc( sizeof(UBaseType_t) );
    configASSERT( semParams[0]->pulSharedVariable );
    configASSERT( semParams[1]->pulSharedVariable );
    *(semParams[0]->pulSharedVariable) = expected_init_shared_var[0];
    *(semParams[1]->pulSharedVariable) = expected_init_shared_var[1];

    // first 2 tasks work with the same parameter structure semParams[0], the last 2 work with semParams[1]
    for (idx=0; idx<NUM_OF_TASKS; idx++) {
        TaskParameters_t tskparams = {
            pvTaskFuncs[idx], &pcTaskName[idx], intgSTACK_SIZE, (void *)semParams[ idx/2 ],
            (uxPriority | portPRIVILEGE_BIT), stackMemSpace[idx],
            // leave MPU regions uninitialized
        };
        // default value to unused MPU regions 
        for(jdx=0; jdx<portNUM_CONFIGURABLE_REGIONS; jdx++)
        {
            tskparams.xRegions[jdx].pvBaseAddress   = NULL;
            tskparams.xRegions[jdx].ulLengthInBytes = 0;
            tskparams.xRegions[jdx].ulParameters    = 0;
        }
        xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tskparams, NULL );
        configASSERT( xState == pdPASS );
    }
} //// end of vStartBinSemphrCase1()


