/*
 * MySPI.h
 *
 *  Created on: 19 Juin. 2021
 *      Author: Thierry Rocacher
 *
 *  ======= CONTEXTE DU LOGICIEL ===================
 *  CIBLE : stm32F103 (Nucléo board)
 *  IDE : KEIL V5.30.0 via fichier .ioc généré par CubeMx 6.2.1 (en standalone) 
 *  CONTEXTE : voilier, pilotage Accéléro ADXL345 par SPI
 *  DEPENDENCES : Le code s'appuie sur les registres du stm32F103RB donc
 *  essentiellement sur stm32F103xB.h via stm32F10xx.h (voir \Drivers\CMSIS\Device\ST\STM32F1xx\Include)
 *  LOCALISATION DES .h :
 *				- stm32F10xx.h et stm32f103xb.h : \Drivers\CMSIS\Device\ST\STM32F1xx\Include
 *				
 *  Le codage s'appuie sur les registres directement.
 *
 *
 *  ======= GENERALITE  ===================
 *  Un seul SPI peut être utlisé à la fois
 *  Le SPI1 est perturbé par la LED2 (PA5), mais doit pouvoir subir les front SCK qd même (LED clignote vite..)
 *  le SPI2 n'est pas utilisable car pin non connectées par défaut. En fait la Nucléo fait un choix entre SPI1 et SPI2
 *  par soudage jumper.
  
 * 	**IO SPI 1**
	SPI1_NSS PA4
	SPI1_SCK PA5
	SPI1_MISO  PA6  
	SPI1_MOSI  PA7  

	**IO SPI 2**
	SPI2_NSS PB12
	SPI2_SCK PB13
	SPI2_MISO  PB14  
	SPI2_MOSI  PB15  


 */







#ifndef INC_MYSPI_H_
#define INC_MYSPI_H_

#include "stm32F1xx.h"



/******************************************************************************
*  Public Functions
******************************************************************************/

/**
  * @brief 
	* @param :
  */
void MySPI_Init(SPI_TypeDef * SPI);


/**
  * @brief 
	* @param :
  */
void MySPI_Send(char ByteToSend);


/**
  * @brief 
	* @param :
  */
char MySPI_Read(void);



/**
  * @brief 
	* @param :
  */
void MySPI_Set_NSS(void);



/**
  * @brief 
	* @param :
  */
void MySPI_Clear_NSS(void);

#endif 
