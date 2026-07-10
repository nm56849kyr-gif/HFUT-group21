#include "bsp_lidar.h"



void USART3_init(u32 baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);//������ӳ�� Partial Reimaging
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);    

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	  
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure); 
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE);  
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //�������ж�   Enable receive interrupt     
	//USART_ClearFlag(USART3,USART_FLAG_TC);
	USART_Cmd(USART3, ENABLE);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}


//����һ���ַ�  Send a character
void USART3_Send_U8(uint8_t ch)
{
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(USART3, ch);
}

//����һ���ַ���  Send a string
/**
 * @Brief: UsART3�������� UsART3 sends data
 * @Note: 
 * @Parm: BufferPtr:�����͵�����  Length:���ݳ���  BufferPtr: data to be sent Length: data length
 * @Retval: 
 */
void USART3_Send_ArrayU8(uint8_t *BufferPtr, uint16_t Length)
{
	while (Length--)
	{
		USART3_Send_U8(*BufferPtr);
		BufferPtr++;
	}
}

//�����жϷ�����  Serial port interrupt service function
// Serial port interrupt service function
void USART3_IRQHandler(void)
{
    uint8_t Rx3_Temp;
    
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        Rx3_Temp = USART_ReceiveData(USART3);
        
        // ==================== 【新增】与51单片机通信接收处理 ====================
        static uint8_t Usart3_Rx_Buffer[128];   // 静态缓冲区
        static uint16_t Usart3_Rx_Cnt = 0;
        
        if (Usart3_Rx_Cnt < 127)
        {
            Usart3_Rx_Buffer[Usart3_Rx_Cnt++] = Rx3_Temp;
            
            // 收到换行符，认为一条指令结束
            if (Rx3_Temp == '\n' || Usart3_Rx_Cnt >= 100)
            {
                Usart3_Rx_Buffer[Usart3_Rx_Cnt] = '\0';
                
                // 在这里处理51发来的指令
                Process_51_Command(Usart3_Rx_Buffer);
                
                Usart3_Rx_Cnt = 0;        // 清空缓冲区，准备接收下一条
            }
        }
        else
        {
            Usart3_Rx_Cnt = 0;   // 防止缓冲区溢出
        }
    }
}
// 处理从51单片机发来的指令
void Process_51_Command(uint8_t *cmd)
{
    // 示例：你可以在这里添加各种指令处理
    if (strstr((char*)cmd, "START") != NULL)
    {
        // 执行启动小车操作...
        USART3_Send_ArrayU8((uint8_t*)"OK:START\r\n", 10);
    }
    else if (strstr((char*)cmd, "STOP") != NULL)
    {
        // 执行停止操作...
        USART3_Send_ArrayU8((uint8_t*)"OK:STOP\r\n", 9);
    }
    else if (strstr((char*)cmd, "HELLO") != NULL)
    {
        USART3_Send_ArrayU8((uint8_t*)"STM32:HELLO 51\r\n", 16);
    }
    // 可以继续添加更多指令...
}


