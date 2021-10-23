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







#ifndef INC_MYTIMER_H_
#define INC_MYTIMER_H_


#include "stm32f10x.h"
#include "Mygpio.h"


typedef struct
{
	TIM_TypeDef * Timer;
	unsigned short ARR;      // 0 à 15
	unsigned short PSC;    // bauds
} MyTimer_Struct_TypeDef;


/******************************************************************************
*  Public Functions
******************************************************************************/



/****************************** CODEUR INC ***************************/
	/*
	  ** TIMER 1 	**  	**Timer 2 **   		  ** Timer 3  **					** Timer 4  **
		CH1 PA8		 			  	CH1	PA0				    			CH1	PA6  						 CH1 PB6
		CH2 PA9					  	CH2 PA1				    			CH2 PA7							 CH1 PB7
	*/

/**
  * @brief 
	* @param :
	* @Note : Le Timer n'est pas lancé. Il faut le faire en suivant avec MyTimer_Base_Start
						Fonction à lance systématiquement avant d'aller plus en détail dans les conf (PWM,
						codeur inc...)
  */
void MyTimer_Base_Init(MyTimer_Struct_TypeDef * Timer);


#define MyTimer_Base_Start(Timer) (Timer->CR1|=TIM_CR1_CEN)
#define MyTimer_Base_Stop(Timer) (Timer->CR1&=~TIM_CR1_CEN)


/**
  * @brief 
	* @param :
	* @Note : Il faut lancer MyTimer_Base_Init avant
						Le Timer est stoppé pour faire la conf. 
						Il faut le lancer après : MyTimer_Base_Start
					  Les IO ch1 et ch2 sont configurées par la fct
  */

void MyTimer_CodeurInc(TIM_TypeDef * Timer, char Resolution);
// Resolution correspond au nbre de fronts comptés par période disque
#define x2 1
#define x4 3


/**
  * @brief 
	* @param :
	* @Note : 
  */
int MyTimer_getCNT(TIM_TypeDef * Timer);



#define PWM_Valeur(Timer,Voie, Val) (Timer->CCR##Voie=Val)





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
void MyTimer_PWM(TIM_TypeDef * Timer, char Channel);


/**
  * @brief 
	* @param :
	* @Note : Il faut lancer MyTimer_Base_Init avant
						Le Timer est stoppé pour faire la conf. 
						Il faut le lancer après : MyTimer_Base_Start
					  Les IO sont configurées par la fct (alt output ppull)
  */
void MyTimer_PWM(TIM_TypeDef * Timer, char Channel);

#endif 
