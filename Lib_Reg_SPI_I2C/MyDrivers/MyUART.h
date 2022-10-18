/*
 * MyUART.h
 *
 *  Created on: 06 Jul 2021
 *      Author: Thierry Rocacher
 *
 *  ======= CONTEXTE DU LOGICIEL ===================
 *  CIBLE : stm32F103 (Nucléo board)
 *  IDE : KEIL V5.30.0 via fichier .ioc généré par CubeMx 6.2.1 (en standalone) 
 *  CONTEXTE : voilier, communication Xbee
 *  DEPENDENCES : Le code s'appuie sur les registres du stm32F103RB donc
 *  essentiellement sur stm32F103xB.h via stm32F10xx.h (voir \Drivers\CMSIS\Device\ST\STM32F1xx\Include)
 *  LOCALISATION DES .h :
 *				- stm32F10xx.h et stm32f103xb.h : \Drivers\CMSIS\Device\ST\STM32F1xx\Include
 *        - core_cm3.h pour tout ce qui est NVIC + fct spéciales
 *        - MyGPIO.h 
 *				
 *  Le codage s'appuie sur les registres directement.
 *
 *
 *  ======= GENERALITE  ===================
 La lib fournit les fonctionnalités de bas niveau de l'UART mais aussi des fonctions
de plus haut niveau qui permettent la manipulation de chaîne de caractère.

* les 3 USART sont programmables
* les IO sont configurées dans la fonction d'initialisation de l'USART

=> USART_1_TX = TIM1_CH2 = PA9
=> USART_1_RX = TIM1_CH3 = PA10
=> USART_2_TX = TIM2_CH3 = PA2
=> USART_2_RX = TIM2_CH4 = PA3
=> USART_3_TX = PB10
=> USART_3_RX = PB11
 
Le débit de chacune des transmission est paramétrable.
Par contre les autres configurations sont figées :
- pas de parité
- 8 bits
- pas de contrôle de flux matériel
- 1 bit de stop


 */










#ifndef INC_USART_H_
#define INC_USART_H_

#include "stm32F1xx.h"


#define CR 0x0D

typedef struct
{
	USART_TypeDef * UART;
	char Prio;      // 0 à 15
	int BaudRate;   // bauds
	void (*Ptr_IT_Fct)(void); // @ de la fonction à appeler lors de la réception octet.
} MyUART_Struct_TypeDef;


/******************************************************************************
*  Public Functions
******************************************************************************/

/**
  * @brief 
	* @param :
  */
void MyUART_Init(MyUART_Struct_TypeDef * UART);


/**
  * @brief 
	* @param :
  */
void MyUART_Send(USART_TypeDef * UartPtr, char ByteToSend);


/**
  * @brief 
	* @param :
  */
void MyUART_SendString(USART_TypeDef * UartPtr, char * str);

/**
  * @brief 
	* @param :
  */
char MyUART_getValue(USART_TypeDef * UartPtr);
	


#endif 
