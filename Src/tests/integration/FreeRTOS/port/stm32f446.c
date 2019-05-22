#include "tests/integration/FreeRTOS/port/stm32f446.h"

extern void  MX_TIM3_Init(void);
extern void  MX_TIM4_Init(void);


void  vIntegrationTestDeviceInit(void)
{
    vFloatRegSetTest(0.0f);
    MX_TIM3_Init();
    MX_TIM4_Init();
}

void vFloatRegSetTest(float fin)
{
    __asm volatile(
        "vmov.F32  s4 , %0  \r\n"
        "vmov.F32  s5 , %0  \r\n"
        "vmov.F32  s6 , %0  \r\n"
        "vmov.F32  s7 , %0  \r\n"
        "vmov.F32  s8 , %0  \r\n"
        "vmov.F32  s9 , %0  \r\n"
        "vmov.F32  s10, %0  \r\n"
        "vmov.F32  s11, %0  \r\n"
        :
        :"Ir"(fin)
        :
    );
}


float fFloatRegGetTest(void)
{
    float fout = 0.0;
    __asm volatile(
        "vmov.F32  %0, s4  \r\n"
        :"=r"(fout) ::
    );
    return fout;
}



StackType_t *pxPortTestRecoverExptnStack( StackType_t *pxTopOfStack, TaskFunction_t pxTaskStartFunc, void *pvParams )
{
    StackType_t   *currSP = pxTopOfStack;
    currSP--;
   *currSP = xPSR_T_Msk;
    currSP--;
   *currSP = ((StackType_t) pxTaskStartFunc) & portCPU_ADDRESS_MASK;
    currSP--;
   *currSP = 0x0;
    currSP--;
    currSP -= 4;
   *currSP = (StackType_t *) pvParams;
    return currSP;
} // end of pxPortTestRecoverExptnStack





void  vPreemptCurrentInterruptTest()
{
    NVIC_SetPendingIRQ(TIM4_IRQn);
}


