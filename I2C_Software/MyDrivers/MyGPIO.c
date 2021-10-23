
#include "MyGPIO.h"

void MyGPIO_Init( MyGPIO_Struct_TypeDef *  GPIOStructPtr)
{

	if (GPIOStructPtr->GPIO==GPIOA) RCC->APB2ENR|= RCC_APB2ENR_IOPAEN;
	else if (GPIOStructPtr->GPIO==GPIOB) RCC->APB2ENR|= RCC_APB2ENR_IOPBEN;	
	else if (GPIOStructPtr->GPIO==GPIOC) RCC->APB2ENR|= RCC_APB2ENR_IOPCEN;	
	else   RCC->APB2ENR|= RCC_APB2ENR_IOPDEN;	// GPIOD
	
	if ( GPIOStructPtr->GPIO_Conf==In_PullDown)
	{
		(GPIOStructPtr->GPIO)->ODR&=~(1<<(GPIOStructPtr->GPIO_Pin)); // mise à 0 du bit corresp
		// dans ODR
		GPIOStructPtr->GPIO_Conf=In_PullDown; // même code CNFMOD pour up ou douwn
	}
	
	else if ( GPIOStructPtr->GPIO_Conf==In_PullUp)
	{
		(GPIOStructPtr->GPIO)->ODR|=1<<(GPIOStructPtr->GPIO_Pin); // mise à 1 du bit corresp
		// dans ODR
		GPIOStructPtr->GPIO_Conf=In_PullDown; // même code CNFMOD pour up ou douwn
	}
	
	// Programmation
	if ((GPIOStructPtr->GPIO_Pin <8)) // CRL
	{	
	  (GPIOStructPtr->GPIO)->CRL&=~(0xF<<((GPIOStructPtr->GPIO_Pin)*4));
		(GPIOStructPtr->GPIO)->CRL|=((GPIOStructPtr->GPIO_Conf)<<((GPIOStructPtr->GPIO_Pin)*4));
	}
	else
	{
	  (GPIOStructPtr->GPIO)->CRH&=~(0xF<<((GPIOStructPtr->GPIO_Pin-8)*4));
		(GPIOStructPtr->GPIO)->CRH|=((GPIOStructPtr->GPIO_Conf)<<((GPIOStructPtr->GPIO_Pin-8)*4));
	}
	
}


int MyGPIO_Read(GPIO_TypeDef * GPIO, char GPIO_Pin) // renvoie 0 ou autre chose différent de 0
{
	return (GPIO->IDR&(1<<GPIO_Pin));
}

void MyGPIO_Set(GPIO_TypeDef * GPIO, char GPIO_Pin)
{
	GPIO->ODR|=(1<<GPIO_Pin);
}

void MyGPIO_Reset(GPIO_TypeDef * GPIO, char GPIO_Pin)
{
	GPIO->ODR&=~(1<<GPIO_Pin);
}
