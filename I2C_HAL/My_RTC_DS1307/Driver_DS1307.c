
/* Includes ------------------------------------------------------------------*/

#include "Driver_DS1307.h"




void DS1307_SetPointer(uint8_t Pteur)
{
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_Slave7bitsAdr, &Pteur,1, HAL_MAX_DELAY);

}


// ================================ Mise � l'heure ==========================

static uint8_t DS1307_TimeString[9]; // r�servation premier byte pteur @
#define PtrAdr 0
#define sec 1
#define min 2
#define hour 3
#define day 4
#define date 5
#define month 6
#define year 7
#define control 8



char Conv_BCD(char Nbre);

void DS1307_SetTime(DS1307_Time_Typedef * UserTime)
{
	// ptr adress � 0
	DS1307_TimeString[PtrAdr]=0;
	// Mise en forme des donn�es BCD, voir documentation
	DS1307_TimeString[sec]=Conv_BCD(UserTime->Sec);
	DS1307_TimeString[min]=Conv_BCD(UserTime->Min);
	DS1307_TimeString[day]=UserTime->Day;
	DS1307_TimeString[date]=Conv_BCD(UserTime->Date);
	DS1307_TimeString[month]=Conv_BCD(UserTime->Month);
	DS1307_TimeString[year]=Conv_BCD(UserTime->Year);
	
	
	// Insertion du bit de contr�le 12h/24h ds le chp Hour (bit6)
	if (UserTime->H_12_Not_24==0) //0-23h59
	{
		DS1307_TimeString[hour]=Conv_BCD(UserTime->Hour);
		(DS1307_TimeString[hour])&=~(1<<6);
	}
	else
	{
		DS1307_TimeString[hour]=Conv_BCD(UserTime->Hour);
		if (UserTime->PM_Not_AM==0) (DS1307_TimeString[hour])&=~(1<<5); // matin
		else (DS1307_TimeString[hour])|=(1<<5); // aprem
		DS1307_TimeString[hour]|=(1<<6);
	}	
	
	// �criture du reg de contr�le, OUT=0 (b7), SQWE=0  (b4)
	//(pas de sortie d'horloge)
	(DS1307_TimeString[control])&=~(1<<4);
	(DS1307_TimeString[control])&=~(1<<7);
	
	
	// �mission effective
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_Slave7bitsAdr, DS1307_TimeString,9, HAL_MAX_DELAY);

	
}


char Conv_BCD(char Nbre)
{
	char u,d;
	d=Nbre/10;
	u=Nbre - 10*d;
	return ((d<<4)|u);
}





// ================================ Lecture de l'heure ==========================



#define BCD_To_Dec(var) (((var)&0x70)>>4)*10+((var)&0x0F)


void DS1307_GetTime(DS1307_Time_Typedef * UserTime)
{
	uint8_t RecTab[7];
	
	DS1307_SetPointer(0); // pointeur adresse du DS1307 � 0 (reg des secondes) pour lect seq
	HAL_I2C_Master_Receive (&hi2c1, DS1307_Slave7bitsAdr, RecTab, 7, HAL_MAX_DELAY);


	
	// remplissage UserTime (supposition format 24h)
	UserTime->Sec=BCD_To_Dec(RecTab[0]);
	UserTime->Min=BCD_To_Dec(RecTab[1]);
	UserTime->Hour=BCD_To_Dec(RecTab[2]&0x3F);
	UserTime->Day=(RecTab[3])&0x07;
	UserTime->Date=BCD_To_Dec(RecTab[4]);
	UserTime->Month=BCD_To_Dec(RecTab[5]);
	UserTime->Year=BCD_To_Dec(RecTab[6]);
}

