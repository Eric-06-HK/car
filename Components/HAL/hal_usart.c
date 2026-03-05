--- Components/HAL/hal_usart.c (原始)
#include "hal_usart.h"

#define DMA1_MEM_LEN 256//DMAÿݴĳ
char _dbg_TXBuff[DMA1_MEM_LEN];

/**************************************************************************USART1 DMA**************************************************************************/

/**************************************************************************
  USART1_SendBuffer
1
                  ʹģʽ,ÿηɺ,װͨ
                  װɡ
ڲbuffer Ĳ  length ĳ
  ֵlength ϣط
**************************************************************************/
uint16_t USART1_SendBuffer(const char* buffer, uint16_t length)
{
        if( (buffer==NULL) || (length==0) )
        {
                return 0;
        }

        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel4, length);
        DMA_Cmd(DMA1_Channel4, ENABLE);
        while(1)
        {
                if(DMA_GetITStatus(DMA1_IT_TC4)!=RESET) //4
                {
                        DMA_ClearFlag(DMA1_IT_TC4);//ͨ4ɱ
                        break;
                }
        }
        return length;
}


/**************************************************************************
  _dbg_printf
DMAʽprintf
ڲformat
  ֵ
**************************************************************************/
void _dbg_printf(const char *format,...)
{
                uint32_t length;
                va_list args;

                va_start(args, format);
                length = vsnprintf((char*)_dbg_TXBuff, sizeof(_dbg_TXBuff), (char*)format, args);
                va_end(args);
                USART1_SendBuffer((const char*)_dbg_TXBuff,length);
}

/**************************************************************************
  HalUASRT1_DMA_Send_init
1 DMA
ڲDMA_CHx  DMA ͨ
                  cpar     DMA
                  cmar     DMA
  ֵ
**************************************************************************/
void HalUASRT1_DMA_Send_init(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar)
{
                DMA_InitTypeDef DMA_InitStructure;

                RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);      //ʹDMA
          DMA_DeInit(DMA_CHx);   //DMA1ĴΪȱʡֵ
                //DMA1_MEM_LEN=cndtr;
                DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA
                DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA
                DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //ݴڴȡ
                DMA_InitStructure.DMA_BufferSize = DMA1_MEM_LEN;  //DMAͨDMA
                DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Ĵ
                DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //ַĴ
                DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //ݿΪ8λ
                DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //ݿΪ8λ
                DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //ģʽ
                DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ xӵ
                DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨxûΪ浽洫
                DMA_Init(DMA_CHx, &DMA_InitStructure);
                USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
                DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
}

/**************************************************************************
  HalUASRT1_DMA_Config
1DMAʼ
                  رRXNETC,IDLE
                  RX,TXĵ
ڲ
  ֵ
**************************************************************************/
void HalUASRT1_DMA_Config(void)
{
                USART_ITConfig(USART1, USART_IT_TC,DISABLE);
                USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
                USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

                HalUASRT1_DMA_Send_init(DMA1_Channel4,(u32)&USART1->DR,(u32)_dbg_TXBuff);//TX
}

/**************************************************************************
  HalUSART1_Init
1
ڲ
  ֵ
**************************************************************************/
void HalUSART1_Init(u32 bound)
{
        USART_InitTypeDef USART_InitStructure;

        //USART ʼ

        USART_InitStructure.USART_BaudRate = bound;//ڲ
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//Ϊ8λݸ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//һֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;//żУλ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //ģʽ

        USART_Init(USART1, &USART_InitStructure); //ʼ
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//ڽж
        USART_Cmd(USART1, ENABLE);                    //ʹ
}

/**************************************************************************
  HalUSART1_IO_Init
1 IOʼ
ڲ
  ֵ
**************************************************************************/
void HalUSART1_IO_Init(void)
{
                GPIO_InitTypeDef GPIO_InitStructure;
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);     //ʹUSART1GPIOAʱ
                        //USART1_TX   GPIOA.9
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
          GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.9

          //USART1_RX     GPIOA.10ʼ
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.10
}

/**************************************************************************USART2 **************************************************************************/

/**************************************************************************
  HalUASRT2_NVIC_Config
2ȼ
ڲ
  ֵ
**************************************************************************/
void HalUASRT2_NVIC_Config(void)
{
        //Usart2 NVIC
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//ռ1
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;              //0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //IRQͨʹ
        NVIC_Init(&NVIC_InitStructure); //ĲʼVICĴ
}

/**************************************************************************
  HalUSART2_Init
2
ڲ
  ֵ
**************************************************************************/
void HalUSART2_Init(u32 bound)
{
        USART_InitTypeDef USART_InitStructure;

        //USART ʼ

        USART_InitStructure.USART_BaudRate = bound;//ڲ
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//Ϊ8λݸ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//һֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;//żУλ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //ģʽ

        USART_Init(USART2, &USART_InitStructure); //ʼ
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//ڽж
        USART_Cmd(USART2, ENABLE);                    //ʹ
}

/**************************************************************************
  HalUSART2_IO_Init
2 IOʼ
ڲ
  ֵ
**************************************************************************/
void HalUSART2_IO_Init(void)
{
          GPIO_InitTypeDef GPIO_InitStructure;
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //ʹUSART2GPIOAʱ
          RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //ʹUSART2GPIOAʱ
                        //USART2_TX   GPIOA.2
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
          GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.2

          //USART2_RX     GPIOA.3ʼ
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.3
}

void HalUARTInit ( void )
{
                HalUSART1_IO_Init();
                HalUSART1_Init(115200);
                HalUASRT1_DMA_Config();

                HalUSART2_IO_Init();
                HalUSART2_Init(115200);
                HalUASRT2_NVIC_Config();
}

+++ Components/HAL/hal_usart.c (修改后)
#include "hal_usart.h"

#define DMA1_MEM_LEN 256//DMAÿݴĳ
char _dbg_TXBuff[DMA1_MEM_LEN];

/**************************************************************************USART1 DMA**************************************************************************/

/**************************************************************************
  USART1_SendBuffer
1
                  ʹģʽ,ÿηɺ,װͨ
                  װɡ
ڲbuffer Ĳ  length ĳ
  ֵlength ϣط
**************************************************************************/
uint16_t USART1_SendBuffer(const char* buffer, uint16_t length)
{
        if( (buffer==NULL) || (length==0) )
        {
                return 0;
        }

        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel4, length);
        DMA_Cmd(DMA1_Channel4, ENABLE);
        while(1)
        {
                if(DMA_GetITStatus(DMA1_IT_TC4)!=RESET) //4
                {
                        DMA_ClearFlag(DMA1_IT_TC4);//ͨ4ɱ
                        break;
                }
        }
        return length;
}


/**************************************************************************
  _dbg_printf
DMAʽprintf
ڲformat
  ֵ
**************************************************************************/
void _dbg_printf(const char *format,...)
{
                uint32_t length;
                va_list args;

                va_start(args, format);
                length = vsnprintf((char*)_dbg_TXBuff, sizeof(_dbg_TXBuff), (char*)format, args);
                va_end(args);
                USART1_SendBuffer((const char*)_dbg_TXBuff,length);
}

/**************************************************************************
  HalUASRT1_DMA_Send_init
1 DMA
ڲDMA_CHx  DMA ͨ
                  cpar     DMA
                  cmar     DMA
  ֵ
**************************************************************************/
void HalUASRT1_DMA_Send_init(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar)
{
                DMA_InitTypeDef DMA_InitStructure;

                RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);      //ʹDMA
          DMA_DeInit(DMA_CHx);   //DMA1ĴΪȱʡֵ
                //DMA1_MEM_LEN=cndtr;
                DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA
                DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA
                DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //ݴڴȡ
                DMA_InitStructure.DMA_BufferSize = DMA1_MEM_LEN;  //DMAͨDMA
                DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //Ĵ
                DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //ַĴ
                DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //ݿΪ8λ
                DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //ݿΪ8λ
                DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //ģʽ
                DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ xӵ
                DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨxûΪ浽洫
                DMA_Init(DMA_CHx, &DMA_InitStructure);
                USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
                DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
}

/**************************************************************************
  HalUASRT1_DMA_Config
1DMAʼ
                  رRXNETC,IDLE
                  RX,TXĵ
ڲ
  ֵ
**************************************************************************/
void HalUASRT1_DMA_Config(void)
{
                USART_ITConfig(USART1, USART_IT_TC,DISABLE);
                USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
                USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

                HalUASRT1_DMA_Send_init(DMA1_Channel4,(u32)&USART1->DR,(u32)_dbg_TXBuff);//TX
}

/**************************************************************************
  HalUSART1_Init
1
ڲ
  ֵ
**************************************************************************/
void HalUSART1_Init(u32 bound)
{
        USART_InitTypeDef USART_InitStructure;

        //USART ʼ

        USART_InitStructure.USART_BaudRate = bound;//ڲ
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//Ϊ8λݸ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//һֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;//żУλ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //ģʽ

        USART_Init(USART1, &USART_InitStructure); //ʼ
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//ڽж
        USART_Cmd(USART1, ENABLE);                    //ʹ
}

/**************************************************************************
  HalUSART1_IO_Init
1 IOʼ
ڲ
  ֵ
**************************************************************************/
void HalUSART1_IO_Init(void)
{
                GPIO_InitTypeDef GPIO_InitStructure;
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);     //ʹUSART1GPIOAʱ
                        //USART1_TX   GPIOA.9
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
          GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.9

          //USART1_RX     GPIOA.10ʼ
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.10
}

/**************************************************************************USART2 **************************************************************************/

/**************************************************************************
  HalUASRT2_NVIC_Config
2ȼ
ڲ
  ֵ
**************************************************************************/
void HalUASRT2_NVIC_Config(void)
{
        //Usart2 NVIC
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//ռ1
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;              //0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //IRQͨʹ
        NVIC_Init(&NVIC_InitStructure); //ĲʼVICĴ
}

/**************************************************************************
  HalUSART2_Init
2
ڲ
  ֵ
**************************************************************************/
void HalUSART2_Init(u32 bound)
{
        USART_InitTypeDef USART_InitStructure;

        //USART ʼ

        USART_InitStructure.USART_BaudRate = bound;//ڲ
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//Ϊ8λݸ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//һֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;//żУλ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //ģʽ

        USART_Init(USART2, &USART_InitStructure); //ʼ
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//ڽж
        USART_Cmd(USART2, ENABLE);                    //ʹ
}

/**************************************************************************
  HalUSART2_IO_Init
2 IOʼ
ڲ
  ֵ
**************************************************************************/
void HalUSART2_IO_Init(void)
{
          GPIO_InitTypeDef GPIO_InitStructure;
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //ʹUSART2GPIOAʱ
          RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //ʹUSART2GPIOAʱ
                        //USART2_TX   GPIOA.2
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
          GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.2

          //USART2_RX     GPIOA.3ʼ
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//
          GPIO_Init(GPIOA, &GPIO_InitStructure);//ʼGPIOA.3
}

void HalUARTInit ( void )
{
                HalUSART1_IO_Init();
                HalUSART1_Init(115200);
                HalUASRT1_DMA_Config();

                HalUSART2_IO_Init();
                HalUSART2_Init(115200);
                HalUASRT2_NVIC_Config();

                // ʼUSART3ɨǹPC10=TX, PC11=RX
                HalUSART3_IO_Init();
                HalUSART3_Init(9600);  // ɨǹ
                HalUASRT3_NVIC_Config();
}

/**************************************************************************USART3 ɨǹ**************************************************************************/

/**************************************************************************
  HalUASRT3_NVIC_Config
3ȼɨǹר
ڲ
  ֵ
**************************************************************************/
void HalUASRT3_NVIC_Config(void)
{
        //Usart3 NVIC  - ʹȼ
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // ռ2ڱ0USART21
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                 // 0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    // IRQͨʹ
        NVIC_Init(&NVIC_InitStructure);                                                    // ĲʼVICĴ
}

/**************************************************************************
  HalUSART3_Init
3ɨǹ
ڲbound:
  ֵ
**************************************************************************/
void HalUSART3_Init(u32 bound)
{
        USART_InitTypeDef USART_InitStructure;

        //USART ʼ
        USART_InitStructure.USART_BaudRate = bound;                                // ڲ
        USART_InitStructure.USART_WordLength = USART_WordLength_8b; // Ϊ8λݸ
        USART_InitStructure.USART_StopBits = USART_StopBits_1;    // һֹͣλ
        USART_InitStructure.USART_Parity = USART_Parity_No;               // żУλ
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // Ӳ
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // ģʽ

        USART_Init(USART3, &USART_InitStructure); // ʼ
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // ڽж
        USART_Cmd(USART3, ENABLE);                                               // ʹ
}

/**************************************************************************
  HalUSART3_IO_Init
3 IOʼPC10=TX, PC11=RX
ڲ
  ֵ
**************************************************************************/
void HalUSART3_IO_Init(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;

        // ʹUSART3GPIOCʱ
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);   // ʹGPIOCʱ
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  // ʹUSART3ʱ

        //USART3_TX   GPIOC.10
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // PC10
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //
        GPIO_Init(GPIOC, &GPIO_InitStructure);                  // ʼGPIOC.10

        //USART3_RX       GPIOC.11ʼ
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; // PC11
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //
        GPIO_Init(GPIOC, &GPIO_InitStructure);          // ʼGPIOC.11

        // PC12ΪػIO˴Ϊɨǹδʹ
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
}





















