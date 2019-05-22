#include "tests/integration/FreeRTOS/sw_timer.h"

// define software timer's period in ticks
#define SW_TIM_PERIOD_TICK  pdMS_TO_TICKS(400)
#define SW_TIM_TIMER_ID     0

static volatile portSHORT  uNumSWTimEvtsRecv ;
// for logging assertion failure
static TestLogger_t    *logger = NULL; 


static void vChkSWTimerCallback( TimerHandle_t xTimer )
{
    const portSHORT maxAllowableMarginTicks = 10;
    TickType_t expectedTickCount = 0;
    TickType_t actualTickCount   = 0;
    // check how accurate the someware timer is, since we created & started the auto-reload
    // software timer before starting the task scheduler, we can roughly estimate current
    // tick count (of RTOS kernel) using internal variable uNumSWTimEvtsRecv & the period for
    // software timer (SW_TIM_PERIOD_TICK) . The estimated value should be very close to
    // xTaskGetTickCount() , the real current tick count in RTOS kernel.
    uNumSWTimEvtsRecv += 1;
    expectedTickCount = (TickType_t) uNumSWTimEvtsRecv * SW_TIM_PERIOD_TICK;
    actualTickCount   = xTaskGetTickCount();
    //// TEST_COUNT_ERROR_GT( expectedTickCount, actualTickCount, error_flag_ptr);
    //// TEST_COUNT_ERROR_LT( (expectedTickCount + maxAllowableMarginTicks), actualTickCount, error_flag_ptr);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32_LOGGER( expectedTickCount, actualTickCount, logger );
    TEST_ASSERT_LESS_OR_EQUAL_UINT32_LOGGER( (expectedTickCount + maxAllowableMarginTicks), actualTickCount, logger );
} //// end of vChkSWTimerCallback()



void vStartSoftwareTimerTest( UBaseType_t uxPriority )
{
    logger = xRegisterNewTestLogger( __FILE__ , "software timer test");
    const portSHORT auto_reload_timer = pdTRUE;
    const portSHORT xDontBlock = 0x0;
    TimerHandle_t   xCheckTimer = NULL;
    // here we create & start the software timer before starting task scheduler,
    // so the only software timer here will transit from dormant state to running state,
    //  when RTOS kernel starts task scheduler
    // (also when tick count in RTOS kernel is zero initially)
    uNumSWTimEvtsRecv = 0;
    xCheckTimer       = xTimerCreate("ChkTimer", SW_TIM_PERIOD_TICK, auto_reload_timer, SW_TIM_TIMER_ID, vChkSWTimerCallback);
    configASSERT( xCheckTimer );
    xTimerStart( xCheckTimer, xDontBlock );
} //// end of vStartSoftwareTimerTest


