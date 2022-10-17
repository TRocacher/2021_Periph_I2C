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


#include "MyADXL345.h"

/******************************************************************************
*  PRIVATE Variables & fct
*
******************************************************************************/

void MyADXL345_SingleWrite(char RegAdr, char Byte);
char MyADXL345_SingleRead(char RegAdr);

	
char InitPassed=0;



/******************************************************************************
*  PUBLIC FUNCTION
*  See spec ADXL345
******************************************************************************/




/**
  * @brief 
  */


void MyADXL345_Init(SPI_TypeDef * SPI)
{

	MySPI_Init(SPI);
	
	
// POWER_CTL | 0 0 |Link| AutoSleep | Measure |Sleep | Waekeup(2) |
//           | 0 0   0     0            0       0        xx       |
//@ = 0x2D Value = 0x00
	MyADXL345_SingleWrite(POWER_CTL,0x00);	

// FIFO mode |FIFO_Mode | Trigger | Samples (5)|
// FIFO mode |10 (bypass) | x | x|		
	MyADXL345_SingleWrite(FIFO_CTL,0x00);	
	
	
// Taux de mesure, BW_rate , low Power = no
// @ = 0x2C, value = 0x09 (50Hz)
	MyADXL345_SingleWrite(BW_RATE,0x09);
	
	
// DATA_FORMAT | SelfTest | SPI | Int_Invert|0|FullRes| Justify|range(2)|
//	             | 0         (4 wires) x     0   1       0(left)  11 (+/16g)|
// 
//@ = 0x31 Value = 0x0	
	MyADXL345_SingleWrite(DATA_FORMAT,0x0B);		

// passage en mode Measure.
// POWER_CTL | 0 0 |Link| AutoSleep | Measure |Sleep | Waekeup(2) |
//           | 0 0   0     0            1       0        xx       |
//@ = 0x2D Value = 0x04,
	MyADXL345_SingleWrite(POWER_CTL,0x08);	
	
	InitPassed=1;
}


char MyADXL345_getID(void)
{
	if (InitPassed==1)
	{
		return MyADXL345_SingleRead(0x0);
	}
	else return 255;
	
}


void MyADXL345_getData(MyADXL345_gData * DataPtr)
{
	/* @0x32 = DATAX0
		 @0x33 = DATAX1
		...
		 @0x37 = DATAZ1
	*/
	int RegAdr;
	short int Val16;
	
	RegAdr=DATAX0|3<<6; // R/W (bit 7) = 1, MB (Multiple Byte , bit 6) = 1
	
	// CS goes low
	MySPI_Clear_NSS();
	// Send Multiple Read cmd with Adress
	MySPI_Send(RegAdr);
  
	// lecture X
	Val16=MySPI_Read();
	Val16=Val16 + ((MySPI_Read())<<8);
	DataPtr->g_X=(float)Val16*0.0039;
	// lecture Y
	Val16=MySPI_Read();
	Val16=Val16 + ((MySPI_Read())<<8);
	DataPtr->g_Y=(float)Val16*0.0039;
	// lecture Z
	Val16=MySPI_Read();
	Val16=Val16 + ((MySPI_Read())<<8);
	DataPtr->g_Z=(float)Val16*0.0039;	
	
	// CS back to high
	MySPI_Set_NSS();
	
	
}



void MyADXL345_OffsetComp(MyADXL345_gData * DataPtr)
{

	signed char val;
	
	val=-(signed char)(DataPtr->g_X/0.0156);
	MyADXL345_SingleWrite(OFSX,val);
	val=-(signed char)(DataPtr->g_Y/0.0156);	
	MyADXL345_SingleWrite(OFSY,val);
	val=-(signed char)(DataPtr->g_Z/0.0156);		
	MyADXL345_SingleWrite(OFSZ,val);
}

/* **************************************************
		Private Fcts 
*****************************************************/

void MyADXL345_SingleWrite(char RegAdr, char Byte)
{
	MySPI_Clear_NSS();
	MySPI_Send(RegAdr);
	MySPI_Send(Byte);
	MySPI_Set_NSS();	
}



char MyADXL345_SingleRead(char RegAdr)
{
	char ReadVal;
	RegAdr=RegAdr|1<<7; // R/W (bit 7) = 1
	MySPI_Clear_NSS();
	MySPI_Send(RegAdr); // envoie de l'adresse avec demande lecture en suivant
	ReadVal=MySPI_Read();
	MySPI_Set_NSS();
  return 	ReadVal;
}





