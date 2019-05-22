#ifndef __STM32F446_TEST_H
#define __STM32F446_TEST_H

#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "FreeRTOS.h"
#undef  MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif


#define  vPortTestModifyStackPointer(in)  __set_PSP(in); __set_MSP(*(StackType_t *)(SCB->VTOR));

#define  vPortTestModifyLinkReg(in)       __asm volatile("mov  lr , %0  \r\n"::"Ir"(in):);

#define  vPortTestReturnFromHandler()     __asm volatile("bx lr  \r\n");

// ------------------------------------------------------------------------------------------
// there are few integration tests that need interrupts to trigger ISR,
// for simplicity we enable the timers interrupt (from stm32f4xx) : TIM3, TIM4
// 
// For stm32f4xx board & STM32CubeMX project:
//    * see MX_TIM3_Init(), MX_TIM4_Init() for initialization steps.
//    * add functions vRegressionTestISR1(), vRegressionTestISR2() 
//      into ISR TIM3_IRQHandler() &  TIM4_IRQHandler() respectively
//
// For nested interrupt tests, 
//     The timer itself is not used, just its interrupt vector to force nesting from software.
//     * TIM3 must have lower priority than TIM4,
//     * both must have priorities higher than configMAX_SYSCALL_INTERRUPT_PRIORITY.
//       (see MX_TIM3_Init(), MX_TIM4_Init() for detail) In this project, 
//           ** TIM3 priority is configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 0x1
//           ** TIM4 priority is configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 0x2
//
//
void  vIntegrationTestDeviceInit(void);

// do simple read / write accesses to floating-point registers
void  vFloatRegSetTest(float fin);

float fFloatRegGetTest(void);

StackType_t *pxPortTestRecoverExptnStack( StackType_t *pxTopOfStack, TaskFunction_t pxTaskStartFunc, void *pvParams ); 


// ------------------------------------------------------------------------------------------------
// The following test function is supposed to be invoked in ISR when handling one interrupt, 
// this function generates an interrupt which has higher priority than current handling interrupt
// , at the point nested interrupt is happening, the target should be able to handle this
// late higher-priority interrupt prior to other lower-priority interrupt(s).
//
// In stm32f446 port for this integration tests, 
//     * we only apply TIM3 & TIM4 interrupts with different priorities
//     * software triggers interrrupt by setting NVIC_ISPR (pending bit of the specific interrupt)
//
void  vPreemptCurrentInterruptTest();

#ifdef __cplusplus
}
#endif // end of extern C { ... }
#endif // end of __STM32F446_TEST_H
