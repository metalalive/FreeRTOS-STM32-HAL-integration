#include "tests/integration/FreeRTOS/queue_case2.h"

#define  NUM_OF_TASKS        2
#define  SHARED_Q_LENGTH     5
// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure, representing size of the allocated block.
#define  intgSTACK_SIZE      (( unsigned portSHORT ) 0x3e)

// the data structure below is used only for passing parameters to the tasks in this tests
typedef struct {
    // the queue used by the tasks
    QueueHandle_t        xQueue;    
    // the blocking time applied in Queue send/receive operations
    TickType_t           xBlockTime;
    // for logging assertion failure
    TestLogger_t        *logger; 
} qCaseParamStruct;

// following can be used to control task 2's behavior since task 2 has
// higher priority.
static volatile TaskHandle_t Qks2recvChkTask;
// to add a bit of complexity, all the items, which are sent to the shared queue, are
// from following arrays
static volatile const portSHORT  possibleQItemValue[SHARED_Q_LENGTH] = {-81 ,13, -5, 63, -47};



// count number of 1's in binary representation of a given 16-bit number
// we apply Brian Kernighan's algorithm to this function.
//
// Quick Note: 
// if the given number is not large (e.g. a 4-bit number) in the application,
// then a lookup table is feasible to achieve constant running time O(1) because you can
// map the small value ot input number to number of 1's in its binary form
//
static unsigned portSHORT Count1sBinarySeq(portSHORT in)
{
    unsigned portSHORT count = 0;
    portSHORT pattern = in;
    while (pattern != 0) {
        pattern = pattern & (pattern - 1) ;
        count++;
    }
    return count;
}



static void vQks2recvChk(void *pvParams)
{
    qCaseParamStruct *qParams  = (qCaseParamStruct *) pvParams;
    QueueHandle_t       xQueue = qParams->xQueue;
    TickType_t      xBlockTime = qParams->xBlockTime;
    TestLogger_t       *logger = qParams->logger;

    portSHORT qItem ;
    BaseType_t QopsStatus ;
    portSHORT actualSequenceItems[SHARED_Q_LENGTH] ;
    portSHORT expectSequenceItems[SHARED_Q_LENGTH] ;
    unsigned portSHORT bitPatternOfQueueSendOpts;
    unsigned portSHORT num1sBitPattern;
    portSHORT qSendToBackIdx  ;
    portSHORT qSendToFrontIdx ;
    unsigned portSHORT idx ;
    unsigned portSHORT jdx ;

    for (;;)
    {
        for (idx=0; idx<(portSHORT)pow(2,SHARED_Q_LENGTH); idx++)
        {
            // suspend itself until task 1 fills the shared queue
            vTaskSuspend(NULL);
            // after the task is woken, the shared queue should be full ...
            TEST_ASSERT_EQUAL_UINT32_LOGGER( SHARED_Q_LENGTH, uxQueueMessagesWaiting(xQueue), logger );
            bitPatternOfQueueSendOpts = idx;
            // calculate number of 1's in bitPatternOfQueueSendOpts,
            num1sBitPattern = Count1sBinarySeq( bitPatternOfQueueSendOpts );
            // create expected sequence of items that will be read from the queue.
            // remind that bit 1 means xQueueSendToFront(), while bit 0 means xQueueSendToBack()
            qSendToBackIdx  = num1sBitPattern;
            qSendToFrontIdx = num1sBitPattern - 1;
            for (jdx=0; jdx<SHARED_Q_LENGTH; jdx++)
            {
                qItem = possibleQItemValue[jdx];
                if((bitPatternOfQueueSendOpts & 0x1) == 0x1) {
                    configASSERT( (qSendToFrontIdx >= 0) );
                    expectSequenceItems[qSendToFrontIdx] = qItem;
                    qSendToFrontIdx--;
                }
                else {
                    configASSERT( (qSendToBackIdx < SHARED_Q_LENGTH) );
                    expectSequenceItems[qSendToBackIdx] = qItem;
                    qSendToBackIdx++;
                }
                bitPatternOfQueueSendOpts = bitPatternOfQueueSendOpts >> 1;
            }
            configASSERT( (bitPatternOfQueueSendOpts == 0) );
            // read out & check items from queue one by one, and compare with expectSequenceItems
            for (jdx=0; jdx<SHARED_Q_LENGTH; jdx++)
            {
                // read the item without removing it from the shared queue
                qItem = 0;
                actualSequenceItems[jdx] = 0;
                QopsStatus = xQueuePeek( xQueue, (void *)&qItem, xBlockTime );
                actualSequenceItems[jdx] = qItem;
                TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, QopsStatus, logger );
                TEST_ASSERT_EQUAL_UINT32_LOGGER( (SHARED_Q_LENGTH - jdx), uxQueueMessagesWaiting(xQueue), logger );
                TEST_ASSERT_EQUAL_INT16_LOGGER( expectSequenceItems[jdx], actualSequenceItems[jdx], logger );
                // read the item and remove it from the shared queue
                qItem = 0;
                actualSequenceItems[jdx] = 0;
                QopsStatus = xQueueReceive( xQueue, (void *)&qItem, xBlockTime );
                actualSequenceItems[jdx] = qItem;
                TEST_ASSERT_EQUAL_INT32_LOGGER(  pdPASS, QopsStatus, logger );
                TEST_ASSERT_EQUAL_UINT32_LOGGER( (SHARED_Q_LENGTH - 1 - jdx), uxQueueMessagesWaiting(xQueue), logger );
                TEST_ASSERT_EQUAL_INT16_LOGGER( expectSequenceItems[jdx], actualSequenceItems[jdx], logger );
            }
        }  //// end of loop through bit patterns of the number ranging from 0 to 2 ^ SHARED_Q_LENGTH
    } //// end of outer infinite loop
} //// end of vQks2recvChk





static void vQks2sender(void *pvParams)
{
    qCaseParamStruct *qParams  = (qCaseParamStruct *) pvParams;
    QueueHandle_t       xQueue = qParams->xQueue;
    TickType_t      xBlockTime = qParams->xBlockTime;
    TestLogger_t       *logger = qParams->logger;

    portSHORT qItem ;
    BaseType_t QopsStatus ;
    unsigned portSHORT bitPatternOfQueueSendOpts;
    unsigned portSHORT idx ;
    unsigned portSHORT jdx ;

    for (;;)
    {
        // check if queue works correctly when sending one item to the front of the queue
        qItem = possibleQItemValue[2];
        xQueueSendToFront( xQueue, (void *)&qItem, xBlockTime );
        TEST_ASSERT_EQUAL_UINT32_LOGGER( 1, uxQueueMessagesWaiting(xQueue), logger );
        qItem = 0;
        QopsStatus = xQueueReceive( xQueue, (void *)&qItem, xBlockTime );
        TEST_ASSERT_EQUAL_UINT32_LOGGER( 0, uxQueueMessagesWaiting(xQueue), logger );
        TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, QopsStatus, logger );
        TEST_ASSERT_EQUAL_INT16_LOGGER( possibleQItemValue[2], qItem, logger );

        // do the same thing again, but we send one item to the back of the queue
        qItem = possibleQItemValue[4];
        xQueueSendToBack( xQueue, (void *)&qItem, xBlockTime );
        TEST_ASSERT_EQUAL_UINT32_LOGGER( 1, uxQueueMessagesWaiting(xQueue), logger );
        qItem = 0;
        QopsStatus = xQueueReceive( xQueue, (void *)&qItem, xBlockTime );
        TEST_ASSERT_EQUAL_UINT32_LOGGER( 0, uxQueueMessagesWaiting(xQueue), logger );
        TEST_ASSERT_EQUAL_INT32_LOGGER( pdPASS, QopsStatus, logger );
        TEST_ASSERT_EQUAL_INT16_LOGGER( possibleQItemValue[4], qItem, logger );

        // from here on, this task fills the shared queue, using xQueueSendToFront() or
        // xQueueSendToBack()  with respect to bit pattern of the variable idx, 
        // bit 1 represents xQueueSendToFront(); while bit 0 represents xQueueSendToBack().
        //
        // For example, when idx is 17 (in decimal form),
        // the last 5 bits of idx is 10001 (in binary form), the sequence of send operations will be like:
        //     xQueueSendToFront() --> xQueueSendToBack() --> xQueueSendToBack() -->
        //     xQueueSendToBack() -->  xQueueSendToFront() 
        // every time this task fills the queue in this way, with different sequence of send operations, once 
        // the shared queue is full, this task resumes the other task (vQks2recvChk), the other task starts
        // reading the sequence of items from the queue to see if the sequence is what we expect.
        //
        // Based on the idea above, since the queue should store at most SHARED_Q_LENGTH items, 
        // the number of possible ways to filling the queue is to multiply 2 SHARED_Q_LENGTH times
        // (2 ** SHARED_Q_LENGTH)
        for (idx=0; idx<(portSHORT)pow(2,SHARED_Q_LENGTH); idx++)
        {
            qItem = 0;
            // get next pattern of the number, in this integration test, we only look at the last
            // 5 bits of bitPatternOfQueueSendOpts
            bitPatternOfQueueSendOpts = idx;
            for (jdx=0; jdx<SHARED_Q_LENGTH; jdx++)
            {
                // always check the LSB (least significant bit, bit 0) of bitPatternOfQueueSendOpts, then
                // discard the LSB by shifting bitPatternOfQueueSendOpts to the right by one bit position.
                // By doing so iteratively, we can sequentially read the the last 5 bits of a pattern.
                qItem = possibleQItemValue[jdx];
                if((bitPatternOfQueueSendOpts & 0x1) == 0x1) {
                    xQueueSendToFront( xQueue, (void *)&qItem, xBlockTime );
                }
                else {
                    xQueueSendToBack( xQueue, (void *)&qItem, xBlockTime );
                }
                TEST_ASSERT_EQUAL_UINT32_LOGGER( (jdx+1), uxQueueMessagesWaiting(xQueue), logger );
                bitPatternOfQueueSendOpts = bitPatternOfQueueSendOpts >> 1;
            }
            // by resuming the other task, this task should be preempted then the other task takes
            // the rest of work then finally empty the queue .
            vTaskResume( Qks2recvChkTask );
        }
    } //// end of outer infinite loop
} //// end of vQks2sender





void vStartQueueTestCase2( UBaseType_t uxPriority )
{
    const portSHORT     xDontBlock      = 0;
    qCaseParamStruct   *pxQparamsToTask[NUM_OF_TASKS] ;
    StackType_t        *stackMemSpace[NUM_OF_TASKS] ;
    QueueHandle_t       xQueue;    
    BaseType_t          xState; 
    unsigned portSHORT  idx;

    xQueue = xQueueCreate( SHARED_Q_LENGTH, (unsigned portBASE_TYPE) sizeof(portSHORT) );
    for (idx=0; idx<NUM_OF_TASKS; idx++) {
        stackMemSpace[idx] = (StackType_t *) pvPortMalloc( sizeof(StackType_t) * intgSTACK_SIZE );
    }
    pxQparamsToTask[0] = (qCaseParamStruct *) pvPortMalloc( sizeof(qCaseParamStruct) );
    pxQparamsToTask[1] = (qCaseParamStruct *) pvPortMalloc( sizeof(qCaseParamStruct) );
    configASSERT( pxQparamsToTask[0] );
    configASSERT( pxQparamsToTask[1] );
    pxQparamsToTask[0]->xBlockTime  = xDontBlock;
    pxQparamsToTask[1]->xBlockTime  = xDontBlock;
    pxQparamsToTask[0]->logger      = xRegisterNewTestLogger( __FILE__ , "Queue test (case 2) -- sender task" );
    pxQparamsToTask[1]->logger      = xRegisterNewTestLogger( __FILE__ , "Queue test (case 2) -- receiver task" );
    pxQparamsToTask[0]->xQueue      = xQueue;
    pxQparamsToTask[1]->xQueue      = xQueue;
    configASSERT( xQueue );
    configASSERT( pxQparamsToTask[0]->logger );
    configASSERT( pxQparamsToTask[1]->logger );

    TaskParameters_t tsk1params = {
        vQks2sender, "Qks2sender", intgSTACK_SIZE, (void *)pxQparamsToTask[0],
        (uxPriority | portPRIVILEGE_BIT), stackMemSpace[0],
        // leave MPU regions uninitialized
    };
    TaskParameters_t tsk2params = {
        vQks2recvChk, "Qks2recvChk", intgSTACK_SIZE, (void *)pxQparamsToTask[1],
        ((uxPriority+1) | portPRIVILEGE_BIT), stackMemSpace[1],
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
    xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tsk2params, &Qks2recvChkTask );
    configASSERT( xState == pdPASS );

} //// end of vStartQue ueTestCase2


