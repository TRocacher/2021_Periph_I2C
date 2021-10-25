/*

 */

#include "Mygpio.h"
#include "MyUART.h"

/******************************************************************************
*  PRIVATE Variables & fct
*
******************************************************************************/
MyGPIO_Struct_TypeDef MyUART_GPIO;


void (*Ptr_IT_Fct_USART1)(void)=0;
void (*Ptr_IT_Fct_USART2)(void)=0;
void (*Ptr_IT_Fct_USART3)(void)=0;

char ReceivedData_1,ReceivedData_2, ReceivedData_3;

/******************************************************************************
*  PUBLIC FUNCTION
*  See RM008.pdf RM
******************************************************************************/


/**
  * @brief 
  */

void MyUART_Init(MyUART_Struct_TypeDef * UART)
{
	
	if (UART->UART==USART1)  
	{
	(RCC->APB2ENR)=(RCC->APB2ENR) | RCC_APB2ENR_USART1EN;
	// USART_1_TX = TIM1_CH2 = PA9
	// USART_1_RX = TIM1_CH3 = PA10
	MyUART_GPIO.GPIO=GPIOA;
	MyUART_GPIO.GPIO_Conf=AltOut_Ppull;
	MyUART_GPIO.GPIO_Pin=9;
	MyGPIO_Init(&MyUART_GPIO);
	
	MyUART_GPIO.GPIO=GPIOA;
	MyUART_GPIO.GPIO_Conf=In_Floating;
	MyUART_GPIO.GPIO_Pin=10;
	MyGPIO_Init(&MyUART_GPIO);	

	Ptr_IT_Fct_USART1=UART->Ptr_IT_Fct;
  __NVIC_EnableIRQ(USART1_IRQn);	
	__NVIC_SetPriority(USART1_IRQn, UART->Prio );
	}

if (UART->UART==USART2)
	{
  (RCC->APB1ENR)=(RCC->APB1ENR) | RCC_APB1ENR_USART2EN;

	// USART_2_TX = TIM2_CH3 = PA2
	// USART_2_RX = TIM2_CH4 = PA3
	MyUART_GPIO.GPIO=GPIOA;
	MyUART_GPIO.GPIO_Conf=AltOut_Ppull;
	MyUART_GPIO.GPIO_Pin=2;
	MyGPIO_Init(&MyUART_GPIO);
	
	MyUART_GPIO.GPIO=GPIOA;
	MyUART_GPIO.GPIO_Conf=In_Floating;
	MyUART_GPIO.GPIO_Pin=3;
	MyGPIO_Init(&MyUART_GPIO);		
	
	Ptr_IT_Fct_USART2=UART->Ptr_IT_Fct;
  __NVIC_EnableIRQ(USART2_IRQn);	
	__NVIC_SetPriority(USART2_IRQn, UART->Prio );
	}
if (UART->UART==USART3)  
	{
	(RCC->APB1ENR)=(RCC->APB1ENR) | RCC_APB1ENR_USART3EN;

	// USART_3_TX = PB10
	// USART_3_RX = PB11
	MyUART_GPIO.GPIO=GPIOB;
	MyUART_GPIO.GPIO_Conf=AltOut_Ppull;
	MyUART_GPIO.GPIO_Pin=10;
	MyGPIO_Init(&MyUART_GPIO);
	
	MyUART_GPIO.GPIO=GPIOB;
	MyUART_GPIO.GPIO_Conf=In_Floating;
	MyUART_GPIO.GPIO_Pin=11;
	MyGPIO_Init(&MyUART_GPIO);	

	Ptr_IT_Fct_USART3=UART->Ptr_IT_Fct;
  __NVIC_EnableIRQ(USART3_IRQn);	
	__NVIC_SetPriority(USART3_IRQn, UART->Prio );
	
	}
	// gestion vitesse de transmission 
	// USART1 sur APB2 à 72MHz
	// USART2 et 3  sur APB à 36MHz
	if (UART->UART==USART1)   UART->UART->BRR=(72000000/(UART->BaudRate));
	else UART->UART->BRR=(36000000/(UART->BaudRate));


	UART->UART->CR1=(UART->UART->CR1)&~(0x1<<12); // 8 bits de data
	UART->UART->CR2=(UART->UART->CR2)&~(0x3<<12); // 1 bit de stop
	UART->UART->CR1=(UART->UART->CR1)|USART_CR1_TE; // Transmit Enable
	UART->UART->CR1=(UART->UART->CR1)|USART_CR1_RE; // receive Enable
	UART->UART->CR1=(UART->UART->CR1)|USART_CR1_RXNEIE; // validation IT locale en réception
	UART->UART->CR1=(UART->UART->CR1)|USART_CR1_UE; // UART On
	

}


/**
  * @brief 
	* @param :
  */
void MyUART_Send(USART_TypeDef * UartPtr, char ByteToSend)
{
	while ((UartPtr->SR&USART_SR_TXE)==0);
	UartPtr->DR=ByteToSend;
}


/**
  * @brief 
	* @param :
  */
void MyUART_SendString(USART_TypeDef * UartPtr, char * str)
{
	while(*str!=0)
	{
		MyUART_Send(UartPtr,*str);
		str++;
	}
}




/**
  * @brief 
	* @param :
  */
char MyUART_getValue(USART_TypeDef * UartPtr)
{
	if (UartPtr==USART1) return ReceivedData_1;
	if (UartPtr==USART2) return ReceivedData_2;
	else return ReceivedData_3;
}






/******************************************************************************
*  PIVATE FUNCTION (IT)
*  
******************************************************************************/


void USART1_IRQHandler(void)
{
	ReceivedData_1=USART1->DR; //RXNE FlagStatus cleared
	if (Ptr_IT_Fct_USART1!=0) Ptr_IT_Fct_USART1();
}

void USART2_IRQHandler(void)
{
	ReceivedData_2=USART2->DR; //RXNE FlagStatus cleared
	if (Ptr_IT_Fct_USART2!=0) Ptr_IT_Fct_USART2();
}

void USART3_IRQHandler(void)
{
	ReceivedData_3=USART3->DR; //RXNE FlagStatus cleared
	if (Ptr_IT_Fct_USART3!=0) Ptr_IT_Fct_USART3();
}
