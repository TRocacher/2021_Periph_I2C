/*
 * MyADXL345.h
 *
 *  Created on: 04 Juillet 2021
 *      Author: Thierry Rocacher
 *
 *  ======= CONTEXTE DU LOGICIEL ===================
 *  CIBLE : stm32F103 (Nucléo board)
 *  IDE : KEIL V5.30.0 via fichier .ioc généré par CubeMx 6.2.1 (en standalone) 
 *  CONTEXTE : voilier, pilotage Accéléro ADXL345 par SPI
 *  DEPENDENCES : Le code s'appuie MySPI.h/.C
 *	NOTE : voir la spécification technique du ADXL345
 *				 Le mode retenu est le plus simpl
 *				- réglage d'un sample rate à 100Hz
 *	 			- FIFO en bypass
 *				- Résolution maximale full scale
 *				- pas d'IT, pas de sleep mode ou power saving
 *				- pas de détection de choc
 *  
 *
 */







#ifndef INC_MYADXL345_H_
#define INC_MYADXL345_H_

#include "stm32f10x.h"
#include "MySPI.h"

/******************************************************************************
*  Registers Define
******************************************************************************/

#define DATAX0 0x32
#define DATAX1 0x33
#define DATAY0 0x34
#define DATAY1 0x35
#define DATAZ0 0x36
#define DATAZ1 0x37

#define OFSX 0x1E
#define OFSY 0x1F
#define OFSZ 0x20

#define DATA_FORMAT 0x31
#define POWER_CTL 0x2D
#define BW_RATE 0x2C
#define FIFO_CTL 0x38

/******************************************************************************
*  Data structures
******************************************************************************/
typedef struct
{
  float g_X;
	float g_Y;
	float g_Z;
} MyADXL345_gData;



/******************************************************************************
*  Public Functions
******************************************************************************/

/**
  * @brief 
	* @param :
  */
void MyADXL345_Init(SPI_TypeDef * SPI);


/**
  * @brief 
	* @param :
  */
char MyADXL345_getID(void);



/**
  * @brief 
	* @param :
  */
void MyADXL345_getData(MyADXL345_gData * DataPtr);



/**
  * @brief 
	* @param :
  */
void MyADXL345_OffsetComp(MyADXL345_gData * DataPtr);





/******************************************************************************
*  Public Functions for direct writing / reading in registers (used for test)
******************************************************************************/

/**
  * @brief 
	* @param :
  */
void MyADXL345_SingleWrite(char RegAdr, char Byte);

/**
  * @brief 
	* @param :
  */
char MyADXL345_SingleRead(char RegAdr);


#endif 
