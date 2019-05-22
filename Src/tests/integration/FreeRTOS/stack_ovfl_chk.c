#include "tests/integration/FreeRTOS/stack_ovfl_chk.h"

// in our case, we create tasks' stack using privileged function  pvPortMalloc() 
// implemented in heap_4.c, each allocated block memory preserves first 8 bytes 
// for  internal data structure.
#define  intgSTACK_SIZE      (( unsigned portSHORT ) 0x3e)

static TaskHandle_t   StkOvflTask_tcb ;
// for logging assertion failure
static TestLogger_t  *logger = NULL; 
static StackType_t   *stackMemPtrbackup ;
volatile static StackType_t   *preserve_space_ovfl ;
static UBaseType_t    expectNumStkOvfls = 0;
static UBaseType_t    actualNumStkOvfls = 0;

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook( TaskHandle_t pxCurrentTCBhandle, const portCHAR *pcTaskName )
{
    // this hook function is called ONLY on stack overflow detected after the victim task is saved
    // to its stack (context switch). It cannot instantly detect stack overflow immediately after
    // a task writes a location out of its stack, to approach instant stack overflow detection,
    // the hardware component like MPU should be applied. 
} //// end of vApplicationStackOverflowHook()
#endif //// end of configCHECK_FOR_STACK_OVERFLOW




static UBaseType_t vRecurFibonacciNum(unsigned portSHORT n )
{
    if(n < 2) {
        return n;
    }
    else {
        return vRecurFibonacciNum(n - 2) + vRecurFibonacciNum(n - 1) ; 
    }
} //// end of vRecurFibonacciNum()




static void vStkOvflTask (void *pvParams)
{
    const portSHORT idx = 0;
    TEST_ASSERT_EQUAL_UINT32_LOGGER( expectNumStkOvfls, actualNumStkOvfls, logger );
    expectNumStkOvfls++;
    // delibarately invoke a recursive function, fill the task's stack space, 
    // until stack overflow is detected by underlying hardware (e.g. MPU, MMU in CPU)
    // then recover stack frames for this user task within RTOS kernel.
    vRecurFibonacciNum( intgSTACK_SIZE );
    // CPU should NOT get here in this test case.
    TEST_ASSERT_EQUAL_UINT32_LOGGER( expectNumStkOvfls, actualNumStkOvfls, logger );
    for(;;);
} //// end of vStkOvflTask()




BaseType_t vStackOvflFaultHandler(void)
{
    BaseType_t alreadyHandled = pdFALSE;
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    // check if the task at which stack overflow happened is StkOvflTask_tcb,
    TaskHandle_t pxCurrentTCBhandle = xTaskGetCurrentTaskHandle();
    if(StkOvflTask_tcb == pxCurrentTCBhandle) {
        // mark as the fault is already processed.
        alreadyHandled = pdTRUE;
        actualNumStkOvfls++;
        // reset SP stack frame for this test
        StackType_t   *currSP = stackMemPtrbackup;
        currSP = pxPortTestRecoverExptnStack( currSP, vStkOvflTask, NULL);
        vPortTestModifyStackPointer( (UBaseType_t)currSP );
        vPortTestModifyLinkReg( (UBaseType_t)portEXPTN_RETURN_TO_TASK );
        vPortTestReturnFromHandler();
    }
#endif //// end of configCHECK_FOR_STACK_OVERFLOW
    return alreadyHandled;
} // end of vStackOvflFaultHandler




void vStartStackOverflowCheck( UBaseType_t uxPriority )
{
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    extern  UBaseType_t  __unpriv_data_start__ [];
    extern  UBaseType_t  __unpriv_data_end__ [];
    StackType_t  *stackMemSpace;
    BaseType_t    xState; 
    portSHORT     idx;

    preserve_space_ovfl = (StackType_t *) pvPortMalloc( sizeof(StackType_t) * 0x1e );

    logger = xRegisterNewTestLogger( __FILE__ , "stack overflow check");
    stackMemSpace = (StackType_t *) pvPortMalloc(sizeof(StackType_t) * intgSTACK_SIZE);
    stackMemPtrbackup = stackMemSpace + intgSTACK_SIZE - 1; 
    stackMemPtrbackup = (StackType_t *) (((portPOINTER_SIZE_TYPE) stackMemPtrbackup ) & (~((portPOINTER_SIZE_TYPE)portBYTE_ALIGNMENT_MASK ))); 
    TaskParameters_t tskparams = {
        vStkOvflTask, "StkOvflTask", intgSTACK_SIZE, NULL,
        uxPriority , stackMemSpace, 
        // leave MPU regions uninitialized
    }; 
    // default value to xRegions 
    for(idx=0; idx<portNUM_CONFIGURABLE_REGIONS; idx++)
    {
        tskparams.xRegions[idx].pvBaseAddress   = NULL;
        tskparams.xRegions[idx].ulLengthInBytes = 0;
        tskparams.xRegions[idx].ulParameters    = 0;
    }
    // add extra region for logging assertion failure in unprivileged tasks.
    tskparams.xRegions[0].pvBaseAddress   = (UBaseType_t) __unpriv_data_start__;
    tskparams.xRegions[0].ulLengthInBytes = (UBaseType_t) __unpriv_data_end__ - (UBaseType_t) __unpriv_data_start__;
    tskparams.xRegions[0].ulParameters    = portMPU_REGION_READ_WRITE | MPU_RASR_S_Msk
                                           | MPU_RASR_C_Msk | MPU_RASR_B_Msk;
    xState = xTaskCreateRestricted( (const TaskParameters_t * const)&tskparams, &StkOvflTask_tcb );
    configASSERT( xState == pdPASS );
#endif
} //// end of vStartStackOverflowCheck()


