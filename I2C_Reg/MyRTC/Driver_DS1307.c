
/* Includes ------------------------------------------------------------------*/

#include "Driver_DS1307.h"
#include "MyI2C.h"


// pour généraliser l'I2C à utiliser
static I2C_TypeDef * I2C_DS1307;



void DS1307_Init(I2C_TypeDef * I2Cx)
{
	I2C_DS1307=I2Cx;


	MyI2C_Init(I2Cx, 2);
}






// ================================ Mise à l'heure ==========================

static char DS1307_TimeString[8]; 

#define sec 0
#define min 1
#define hour 2
#define day 3
#define date 4
#define month 5
#define year 6
#define control 7

static MyI2C_RecSendData_Typedef MyI2C_SendTimeData;

static char Conv_BCD(char Nbre);

void DS1307_SetTime(DS1307_Time_Typedef * UserTime)
{

	// Mise en forme des données BCD, voir documentation
	DS1307_TimeString[sec]=Conv_BCD(UserTime->Sec);
	DS1307_TimeString[min]=Conv_BCD(UserTime->Min);
	DS1307_TimeString[day]=UserTime->Day;
	DS1307_TimeString[date]=Conv_BCD(UserTime->Date);
	DS1307_TimeString[month]=Conv_BCD(UserTime->Month);
	DS1307_TimeString[year]=Conv_BCD(UserTime->Year);
	
	
	// Insertion du bit de contrôle 12h/24h ds le chp Hour (bit6)
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
	
	// écriture du reg de contrôle, OUT=0 (b7), SQWE=0  (b4)
	//(pas de sortie d'horloge)
	(DS1307_TimeString[control])&=~(1<<4);
	(DS1307_TimeString[control])&=~(1<<7);
	
	
	// Préparation émission i2C
	MyI2C_SendTimeData.Nb_Data=8;
	MyI2C_SendTimeData.Ptr_Data=DS1307_TimeString;
	MyI2C_SendTimeData.SlaveAdress7bits=DS1307_Slave7bitsAdr;
	
	// émission effective
	MyI2C_PutString(I2C_DS1307,0, &MyI2C_SendTimeData);
	
}


char Conv_BCD(char Nbre)
{
	char u,d;
	d=Nbre/10;
	u=Nbre - 10*d;
	return ((d<<4)|u);
}





// ================================ Lecture de l'heure ==========================

static MyI2C_RecSendData_Typedef MyI2C_RecevievedTimeData;

#define BCD_To_Dec(var) (((var)&0x70)>>4)*10+((var)&0x0F)

static char RecTab[7];

void DS1307_GetTime(DS1307_Time_Typedef * UserTime)
{

	
	MyI2C_RecevievedTimeData.Nb_Data=7;
	MyI2C_RecevievedTimeData.Ptr_Data=RecTab;
	MyI2C_RecevievedTimeData.SlaveAdress7bits=DS1307_Slave7bitsAdr;
	
	
	MyI2C_GetString(I2C_DS1307,0x0, &MyI2C_RecevievedTimeData);
	
	// remplissage UserTime (supposition format 24h)
	UserTime->Sec=BCD_To_Dec(RecTab[0]);
	UserTime->Min=BCD_To_Dec(RecTab[1]);
	UserTime->Hour=BCD_To_Dec(RecTab[2]&0x3F);
	UserTime->Day=(RecTab[3])&0x07;
	UserTime->Date=BCD_To_Dec(RecTab[4]);
	UserTime->Month=BCD_To_Dec(RecTab[5]);
	UserTime->Year=BCD_To_Dec(RecTab[6]);
}

