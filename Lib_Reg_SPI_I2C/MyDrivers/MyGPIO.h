#ifndef MYGPIO_H
#define MYGPIO_H

#include "stm32f10x.h"

typedef struct
{
	GPIO_TypeDef * GPIO;
	char GPIO_Pin;    // numéro de pin 0 à 15
	char GPIO_Conf;   // see below
} MyGPIO_Struct_TypeDef;

#define In_Floating   0x4  //   0100
#define In_PullDown		0x8  //   1000 pdwn
#define In_PullUp     0x9  //   1001 !!! code spécial pour pull up
#define In_Analog		  0    //   0000 
#define Out_Ppull 	  0x2  //   0010
#define Out_OD        0x6  //   0110
#define AltOut_Ppull  0xA  //   1010
#define AltOut_OD			0xE  //   1110

// CNF MOD
// xx xx 

void MyGPIO_Init( MyGPIO_Struct_TypeDef *  GPIOStructPtr);
int MyGPIO_Read(GPIO_TypeDef * GPIO, char GPIO_Pin); // renvoie 0 ou autre chose différent de 0
void MyGPIO_Set(GPIO_TypeDef * GPIO, char GPIO_Pin);
void MyGPIO_Reset(GPIO_TypeDef * GPIO, char GPIO_Pin);
#endif
