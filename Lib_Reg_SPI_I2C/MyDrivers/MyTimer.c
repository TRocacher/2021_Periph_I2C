/*
 * MyTimer.h
 *
 *  Created on: 7 Juilet. 2021
 *      Author: Thierry Rocacher
 *
 *  ======= CONTEXTE DU LOGICIEL ===================
 *  CIBLE : stm32F103 (Nucléo board)
 *  IDE : KEIL V5.30.0 via fichier .ioc généré par CubeMx 6.2.1 (en standalone) 
 *  CONTEXTE : voilier, pilotage codeur ing et servo moteur
 *  DEPENDENCES : Le code s'appuie sur les registres du stm32F103RB donc
 *  essentiellement sur stm32F103xB.h via stm32F10xx.h (voir \Drivers\CMSIS\Device\ST\STM32F1xx\Include)
 *  LOCALISATION DES .h :
 *				- stm32F10xx.h et stm32f103xb.h : \Drivers\CMSIS\Device\ST\STM32F1xx\Include
 *			  - core_cm3.h pour tout ce qui est NVIC + fct spéciales
 *        - MyGPIO.h 
 *  Le codage s'appuie sur les registres directement.
 *
 */


#include "MyTimer.h"

/******************************************************************************
*  PRIVATE Variables & fct
*
******************************************************************************/
MyGPIO_Struct_TypeDef MytimerGPIO;



/******************************************************************************
*  PUBLIC FUNCTION
*  See RM008.pdf RM
******************************************************************************/


/**
  * @brief 
  */
void MyTimer_Base_Init(MyTimer_Struct_TypeDef * Timer)
{
	// activation horloge periph :
	if (Timer->Timer==TIM1)  (RCC->APB2ENR)|=RCC_APB2ENR_TIM1EN;
	else if (Timer->Timer==TIM2)  (RCC->APB1ENR)|=RCC_APB1ENR_TIM2EN;
	else if (Timer->Timer==TIM3)  (RCC->APB1ENR)|= RCC_APB1ENR_TIM3EN;
	else (RCC->APB1ENR) |= RCC_APB1ENR_TIM4EN;
	MyTimer_Base_Stop(Timer->Timer);	
	Timer->Timer->ARR=Timer->ARR;
	Timer->Timer->PSC=Timer->PSC;
	
}
	

/****************************** CODEUR INC ***************************/
/**
  * @brief 
	* @param :
  */
void MyTimer_CodeurInc(TIM_TypeDef * Timer, char Resolution)
{

	MyTimer_Base_Stop(Timer);
	
	// Mise en place GPIO ch1 et 2 :
	/*
	  ** TIMER 1 	**  	**Timer 2 **   		  ** Timer 3  **					** Timer 4  **
		CH1 PA8		 			  	CH1	PA0				    			CH1	PA6  						 CH1 PB6
		CH2 PA9					  	CH2 PA1				    			CH2 PA7							 CH1 PB7
	*/
	
	if (Timer==TIM1)  
	{
		MytimerGPIO.GPIO=GPIOA;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=8;
		MyGPIO_Init(&MytimerGPIO);
		
		MytimerGPIO.GPIO=GPIOA;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=9;
		MyGPIO_Init(&MytimerGPIO);
	}
	else if (Timer==TIM2)  
	{
		MytimerGPIO.GPIO=GPIOA;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=0;
		MyGPIO_Init(&MytimerGPIO);
		
		MytimerGPIO.GPIO=GPIOA;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=1;
		MyGPIO_Init(&MytimerGPIO);
	}
	else if (Timer==TIM3) 
	{
		MytimerGPIO.GPIO=GPIOA;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=6;
		MyGPIO_Init(&MytimerGPIO);
		
		MytimerGPIO.GPIO=GPIOA;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=7;
		MyGPIO_Init(&MytimerGPIO);
	}
	
	else 
	{
		MytimerGPIO.GPIO=GPIOB;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=6;
		MyGPIO_Init(&MytimerGPIO);
		
		MytimerGPIO.GPIO=GPIOB;
		MytimerGPIO.GPIO_Conf=In_Floating;
		MytimerGPIO.GPIO_Pin=7;
		MyGPIO_Init(&MytimerGPIO);
	}

	// Resolution de comptage : TIMx_SMCR
	// Bits 2:0 SMS:001: Encoder mode 1 - Counter counts up/down on TI2FP2 edge depending on TI1FP1
	//level.
	//Bits 2:0 SMS:010: Encoder mode 2 - Counter counts up/down on TI1FP1 edge depending on TI2FP2
	//level. NON IMPLEMENTE
	//Bits 2:0 SMS:011: Encoder mode 3 - Counter counts up/down on both TI1FP1 and TI2FP2 edges
	//depending on the level of the other input.
	Timer->SMCR&=(~0x0007);
	Timer->SMCR|=Resolution;
	
	// Mapping des broches sur les fils internes de cptages (CCMR1):
	//  ch1 et ch2 vers resp. ICxFP1 et ICxFP2 (champ CC1S et CC2S à 1, Bits 9:8 CC2S[1:0],	Bits 1:0 CC1S
	Timer->CCMR1&=~(0x03); 
	Timer->CCMR1|=1;
	Timer->CCMR1&=(0x03<<8); // masque = ~0000 0000 0111 0000=FF8F : mise à 0 du champ
	Timer->CCMR1|=(1<<8);

	//polarité positive (CC1P et CC2P à 0 de CCER - Bit 5 CC2P,  Bit 1 CC1P
	Timer->CCER&=~(1<<1);
	Timer->CCER&=~(1<<5);

	//TIMx_CR1=	|xxx... CKD[1:0] ARPE CMS DIR OPM URS UDIS CEN |
	// CEN=1 Counter enable
	Timer->CR1&=~0x0001;
	Timer->CR1|=0x0001;
}

int MyTimer_getCNT(TIM_TypeDef * Timer)
{
	return Timer->CNT;
}




/****************************** PWM ***************************/
//  Mapping des IO:
//  	TIM1_CH1 - PA08       TM2_CH1_ETR - PA0 TM3_CH1 - PA6		TIM4_CH1 - PB6	
//    TIM1_CH2 - PA09       TM2_CH2 - PA1		  TM3_CH2 - PA7		TIM4_CH2 - PB7
// 		TIM1_CH3 - PA10       TM2_CH3 - PA2			TM3_CH3 - PB0		TIM4_CH3 - PB8
// 		TIM1_CH4 - PA11		    TM2_CH4 - PA3			TM3_CH4 - PB1		TIM4_CH4 - PB9
//	  TIM1_CH1N - PB13
//	  TIM1_CH2N - PB14
//	  TIM1_CH3N - PB15
/**
  * @brief 
	* @param :
	* @Note : Il faut lancer MyTimer_Base_Init avant
						Le Timer est stoppé pour faire la conf. 
						Il faut le lancer après : MyTimer_Base_Start
					  Les IO sont configurées par la fct (alt output ppull)
  */
void MyTimer_PWM(TIM_TypeDef * Timer, char Channel)
{
	MyTimer_Base_Stop(Timer);
	// GPIO alt output
	MytimerGPIO.GPIO_Conf=AltOut_Ppull;
	if (Timer==TIM1)
	{
		MytimerGPIO.GPIO=GPIOA;
		Timer->BDTR|=TIM_BDTR_MOE; // spécifique au Timer 1
		if (Channel==1) 	MytimerGPIO.GPIO_Pin=8;
		if (Channel==2) 	MytimerGPIO.GPIO_Pin=9;
		if (Channel==3) 	MytimerGPIO.GPIO_Pin=10;
		if (Channel==4) 	MytimerGPIO.GPIO_Pin=11;
	}
	else if (Timer==TIM2)
		{
		MytimerGPIO.GPIO=GPIOA;
		if (Channel==1) 	MytimerGPIO.GPIO_Pin=0;
		if (Channel==2) 	MytimerGPIO.GPIO_Pin=1;
		if (Channel==3) 	MytimerGPIO.GPIO_Pin=2;
		if (Channel==4) 	MytimerGPIO.GPIO_Pin=3;
	}
	else if (Timer==TIM3)
		{
		MytimerGPIO.GPIO=GPIOA;
		if (Channel==1) 	{MytimerGPIO.GPIO=GPIOA;MytimerGPIO.GPIO_Pin=6;}
		if (Channel==2) 	{MytimerGPIO.GPIO=GPIOA;MytimerGPIO.GPIO_Pin=7;}
		if (Channel==3) 	{MytimerGPIO.GPIO=GPIOB;MytimerGPIO.GPIO_Pin=0;}
		if (Channel==4) 	{MytimerGPIO.GPIO=GPIOB;MytimerGPIO.GPIO_Pin=1;}
	}
	else // tim4	
	{
		MytimerGPIO.GPIO=GPIOB;
		if (Channel==1) 	MytimerGPIO.GPIO_Pin=6;
		if (Channel==2) 	MytimerGPIO.GPIO_Pin=7;
		if (Channel==3) 	MytimerGPIO.GPIO_Pin=8;
		if (Channel==4) 	MytimerGPIO.GPIO_Pin=9;
	}
	MyGPIO_Init(&MytimerGPIO);
	
	// config Timer
	if ((Channel==1) || (Channel==2)) 
	{
		//Config OCy en sortie, implicite mais bon :
		Timer->CCMR1 = (Timer->CCMR1)& ~(0x03<<((Channel-1)*8));	//00 ds le champs CCyS
		//PWM en mode Edge aligned OCyM=110
		Timer->CCMR1 = (Timer->CCMR1)& ~(0x07<<((Channel-1)*8+4));
		Timer->CCMR1 = (Timer->CCMR1)|(0x06<<((Channel-1)*8+4));	
		}
	if ((Channel==3) || (Channel==4)) 
		{
		//Config OCy en sortie, implicite mais bon :
		Timer->CCMR2 = (Timer->CCMR2)& ~(0x03<<((Channel-3)*8));	//00 ds le champs CCyS
		//PWM en mode Edge aligned OCyM=110
		Timer->CCMR2 = (Timer->CCMR2)& ~(0x07<<((Channel-3)*8+4));
		Timer->CCMR2 = (Timer->CCMR2)|(0x06<<((Channel-3)*8+4));	
		}
		// polarité positive bit CCyP = 0, reg CCER
		Timer->CCER = (Timer->CCER)& ~(0x01<<((Channel-1)*4+1));
		// validation du canal y, bit CCyE, reg CCER
		Timer->CCER = (Timer->CCER)|(0x01<<((Channel-1)*4));
}





