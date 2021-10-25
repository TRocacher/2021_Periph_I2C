#include "Driver_DS1307.h"
#include "stm32f10x.h"


//************** Var RTC DS1307 *******************************************************
DS1307_Time_Typedef UserTimeSet;

int tps;
int main(void)
{
	
  //************** Configuration RTC DS1307 par I2C *********************************************		
	// initialisation du driver I2C qui va piloter la RTC

	DS1307_Init(I2C1);
	
	// initialisation de la RTC avec une date complète
	UserTimeSet.Sec=5;
	UserTimeSet.Min=28;
	UserTimeSet.H_12_Not_24=0;
	UserTimeSet.PM_Not_AM=0; // on s'en moque ici. Utile que si H12_Not_24=1;
	UserTimeSet.Hour=14;
	UserTimeSet.Day=1;
	UserTimeSet.Date=25;
	UserTimeSet.Month=10;
	UserTimeSet.Year=21;

	DS1307_SetTime(&UserTimeSet);

	
	
  while (1)
  {
		/*******************************************************************************************
		Debug système, passer en réel et ajouter dans watch window :
		--> ADXL345 						--> gData , +/-1.0g
		--> Xbee (UART1) 				--> XbeeReceivedByte (pour voir les octets en réception)
														--> terminal sur xbee destinataire
		--> RTC DS1307(I2C1)    --> UserTimeSet
		--> Girouette						--> Angle_Degre_PlusMoins359  : +/-359
		--> Plateau						  --> Vitesse_PlusMoins99 :  -99 et +99
		--> Servo							  --> ServoVal_0to20 :  0 à 20 qui done 1 à 2 ms soit 5 à 10%
		**************************************************************************************/
		
	
		DS1307_GetTime(&UserTimeSet);
	
		
		// delai
		for (tps=0;tps<100000;tps++)
		{
		}
	  		
  
  }
 
}
