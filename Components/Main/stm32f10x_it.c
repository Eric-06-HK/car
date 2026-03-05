/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
//#include "OSAL_Comdef.h"
//#include "hal_spi.h"
//#include "hal_flash.h"
//#include "hal_led.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "Generic.h"

//#include "uwb.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

extern int Wait_time;
extern uint8_t Prevent_send_bit;
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{

}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{



        /* Go to infinite loop when Hard Fault exception occurs */
        while (1)
        {
                NVIC_SystemReset(); //λ
        }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
        /* o to infinite loop when Memory Manage exception occurs */
        while (1)
        {
//              NVIC_SystemReset(); //λ
        }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{


        /* Go to infinite loop when Bus Fault exception occurs */
        while (1)
        {
//              NVIC_SystemReset(); //λ
        }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{

        /* Go to infinite loop when Usage Fault exception occurs */
        while (1)
        {
//              NVIC_SystemReset(); //λ
        }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
extern void HalDelayTime_Counter(void);
void SysTick_Handler(void)
{
        static int count = 0;
        local_time32_incr++;//ϵͳʱ
        HalDelayTime_Counter();

                if(local_time32_incr%3000 == 0)//1s
        {
                        HalAdcGetState();
        }


        if(local_time32_incr%100 == 0)//100ms
        {
                if(sys_data.eth_rst_time != 0)                                                          //10s ָģʽģ
                {

                        if(--sys_data.eth_rst_time == 0)
                        {
                                USART4_printf("λ\r\n");
                                HalEth_Rst_Off();
                        }
                }
                else
                {
                        LED_flicker(count++,10);
                }
        }
}


void DMA1_Channel4_IRQHandler(void)
{
                if(DMA_GetFlagStatus(DMA1_FLAG_TC4))
        {
                if(Prevent_send_bit==1)//1ʼ
                {
                        Prevent_send_bit=3;//ʼ
                        Wait_time=portGetTickCnt();
//                      USART4_printf(" %d\r\n",Wait_time);//ظ
                        //ʼ
                }

                sys_data.Data_UART_Busy=true;
                DMA_Cmd(DMA1_Channel4, DISABLE);//ݴɣرDMA4ͨ
                DMA_ClearFlag(DMA1_FLAG_TC4);   //DMA1ͨ7
                USART_ClearFlag(USART1,USART_FLAG_TC);
                USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE); //رUSARTDMA
        }
}
void EXTI15_10_IRQHandler(void)
{
        if(EXTI_GetITStatus(HAL_KEY3_EXTI_LIN) != RESET)
        {
                if(GPIO_ReadInputDataBit(HAL_KEY3_PORT, HAL_KEY3_PIN) == 0){
                        HalKey3_IT_Disable();//رж
                        HalTimer4_IT_Enable();//ʱ1
                }
                /* Clear EXTI Line 9 Pending Bit */
                EXTI_ClearITPendingBit(HAL_KEY3_EXTI_LIN);
        }
                if(EXTI_GetITStatus(HAL_KEY2_EXTI_LIN) != RESET)
        {
                if(GPIO_ReadInputDataBit(HAL_KEY2_PORT, HAL_KEY2_PIN) == 0){
//                      HalKey3_IT_Disable();//رж
//                      HalTimer4_IT_Enable();//ʱ1
//                      USART4_printf("\r\n");
                }
                /* Clear EXTI Line 9 Pending Bit */
                EXTI_ClearITPendingBit(HAL_KEY2_EXTI_LIN);
        }



}

/**
 * @brief  USART3Ϸɨǹݽգ
 */
#include "shopping_cart.h"
void USART3_IRQHandler(void)
{
        if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
        {
                uint8_t data = USART_ReceiveData(USART3);
                // ݴݸɨǹ
                BarcodeScanner_OnDataReceived(data);
                USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        }
}






