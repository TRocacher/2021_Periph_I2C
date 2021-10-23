#include "stm32f10x.h"
#include "Driver_DS1307.h"



/*
test I2C

		
*/
//************** Var RTC DS1307 *******************************************************
DS1307_Time_Typedef UserTimeSet;


int main (void)
{
		// initialisation de la RTC avec une date complète
	UserTimeSet.Sec=0;
	UserTimeSet.Min=13;
	UserTimeSet.H_12_Not_24=0;
	UserTimeSet.PM_Not_AM=0; // on s'en moque ici. Utile que si H12_Not_24=1;
	UserTimeSet.Hour=15;
	UserTimeSet.Day=4;
	UserTimeSet.Date=8;
	UserTimeSet.Month=7;
	UserTimeSet.Year=21;
	
	DS1307_Init();
  DS1307_SetTime(&UserTimeSet);
	while(1)
	{
	}
	
}


