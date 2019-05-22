#include "tests/integration/FreeRTOS/nestISR.h"

// external routines / variables --------------------------

// internal variables -------------------------------------

static volatile unsigned int uNumNestInterrupts;
// uMaxNestInterrupts is only used for recording maximum value of uNumNestInterrupts
// discovered in vTIM4regressionFromISR()
static volatile unsigned int uMaxNestInterrupts;
static TestLogger_t* tlogger[2];


void vSetupNestedInterruptTest()
{
    tlogger[0] = xRegisterNewTestLogger( __FILE__ , "for ISR1" );
    tlogger[1] = xRegisterNewTestLogger( __FILE__ , "for ISR2" );
    uNumNestInterrupts = 0;
    uMaxNestInterrupts = 0;
    vIntegrationTestDeviceInit();
}



void vNestInterruptTestISR1()
{
    float reg_read = 0.0f;
    const float    EXPECTED_VALUE = 2.7181f;
    uNumNestInterrupts++;
    vFloatRegSetTest(EXPECTED_VALUE);
    vPreemptCurrentInterruptTest();
    reg_read = fFloatRegGetTest();
    // error happened if the values are different
    TEST_ASSERT_EQUAL_FLOAT_LOGGER( EXPECTED_VALUE, reg_read, tlogger[0] );
    uNumNestInterrupts--;
}



void vNestInterruptTestISR2()
{
    const portSHORT MAX_NUM_NEST_INTERRUPT = 2;
    const float     EXPECTED_VALUE = 3.1415f;
    uNumNestInterrupts++;
    if(uMaxNestInterrupts < uNumNestInterrupts) {
        uMaxNestInterrupts = uNumNestInterrupts;
    }
    TEST_ASSERT_LESS_OR_EQUAL_UINT_LOGGER( MAX_NUM_NEST_INTERRUPT, uMaxNestInterrupts, tlogger[1] );
    vFloatRegSetTest(EXPECTED_VALUE);
    uNumNestInterrupts--;
}



void vNestInterruptTestTickHook()
{
    vFloatRegSetTest(0.377f);
}



