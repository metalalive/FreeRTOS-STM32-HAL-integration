#ifndef __NEST_ISR_H
#define __NEST_ISR_H

#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "unity_fixture.h"
#include "tests/integration/FreeRTOS/port/stm32f446.h"

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------------------------------------------
// this test applies 2 interrupts, one interrupt must have higher priority than another,
// this test will check :
//
//     * the depth of nested interrupts should be at most 2 at any point of time,
//       when lower-priority interrupt (ISR1) is preempted by higher-priority
//       interrupt (ISR2), the depth of nested interrupts must be exactly 2,
//       if the test detect the value greater than 2, then there must be something wrong,
//       error_flag will be asserted
//
//     * there will be 2 ISRs for ISR1 and ISR2,
//       they are vNestInterruptTestISR1() and vNestInterruptTestISR2(),
//       each calls a test function with different input parameter, writes the parameter to
//       few floating-point registers 
//       , since preemption by ISR2 would happen in ISR1, ISR1 must check
//       the value of the floating-point registers after being preempted by ISR2 .
//

void vSetupNestedInterruptTest();

void vNestInterruptTestISR1();

void vNestInterruptTestISR2();

void vNestInterruptTestTickHook();


#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __NEST_ISR_H
