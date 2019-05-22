#include "tests/integration/FreeRTOS/test_runner.h"



void vCreateAllTestTasks( void )
{
    vSetupNestedInterruptTest();
    vStartIntegerMathTasks( tskIDLE_PRIORITY );
    vStartDynamicPriorityCase2( tskIDLE_PRIORITY );
    vStartDynamicPriorityCase1( tskIDLE_PRIORITY );
    #if (configCHECK_FOR_STACK_OVERFLOW > 0)
        vStartStackOverflowCheck( tskIDLE_PRIORITY );
    #endif //// end of configCHECK_FOR_STACK_OVERFLOW
    vStartBlockTimeTasks( configMAX_PRIORITIES - 4 );
    vStartQueueTestCase1( tskIDLE_PRIORITY );
    vStartQueueTestCase2( tskIDLE_PRIORITY );
    vStartQueueTestCase3( tskIDLE_PRIORITY + 1 );
    vStartBinSemphrCase1( tskIDLE_PRIORITY );
    vStartBinSemphrCase2( configMAX_PRIORITIES - 4 );

    #if (configUSE_COUNTING_SEMAPHORES == 1)
        vStartCountSemphrTest( tskIDLE_PRIORITY );
    #endif // end of configUSE_COUNTING_SEMAPHORES

    #if (configUSE_MUTEXES == 1)
        vStartMutexTestCase1( tskIDLE_PRIORITY );
        #if (configUSE_RECURSIVE_MUTEXES == 1)
            vStartRecurMutexTest( tskIDLE_PRIORITY );
        #endif // end of configUSE_RECURSIVE_MUTEXES
    #endif // end of configUSE_MUTEXES 

    #if (configUSE_TASK_NOTIFICATIONS == 1)
        vStartNotifyTaskTest( tskIDLE_PRIORITY );
    #endif //// end of configUSE_TASK_NOTIFICATIONS

    #if (configUSE_TIMERS == 1)
        vStartSoftwareTimerTest( tskIDLE_PRIORITY );
    #endif //// end of configUSE_TIMERS
} // end of vCreateAllTestTasks



void vIntegrationTestRTOSISR1(void)
{
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        vNestInterruptTestISR1();
        vQueueTestCase3ISR1();
        vBinSemphrCase2ISR1();
    }
} // end of vIntegrationTestRTOSISR1



void vIntegrationTestRTOSISR2(void)
{
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        vNestInterruptTestISR2();
        vQueueTestCase3ISR2();
        #if (configUSE_TASK_NOTIFICATIONS == 1)
            vNotifyTestISR( );
        #endif //// end of configUSE_TASK_NOTIFICATIONS
    }
} // end of vIntegrationTestRTOSISR2



BaseType_t vIntegrationTestRTOSMemManageHandler(void)
{
    BaseType_t alreadyHandled = pdFALSE;
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        #if (configCHECK_FOR_STACK_OVERFLOW > 0)
            alreadyHandled = vStackOvflFaultHandler();
        #endif //// end of configCHECK_FOR_STACK_OVERFLOW
    }
    return alreadyHandled;
} // end of vIntegrationTestRTOSMemManageHandler



#if (configUSE_TICK_HOOK > 0)
void vApplicationTickHook( void )
{
    vNestInterruptTestTickHook();
}
#endif // end of configUSE_TICK_HOOK




