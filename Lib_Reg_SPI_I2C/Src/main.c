#include "Driver_DS1307.h"
#include "stm32f10x.h"
#include "Driver_LTC2944.h"
#include "MyADXL345.h"

//#define UseLTC2944
//#define UseDS1307
#define UseADXL345

//************** Var Accelero AXL345 *****************************************************
#ifdef UseADXL345
MyADXL345_gData User_g;
int ID;
#endif

//************** Var RTC DS1307 *******************************************************
#ifdef UseDS1307
DS1307_Time_Typedef UserTimeSet;
#endif
//************** Var Gauge LTC2944 *******************************************************

#ifdef UseLTC2944
LTC2944_Conf_Typedef UserLTC2944Conf;
LTC2944_AnalogVal_Typedef LTC2944AnalogStruct;
float Q;
#endif



int tps;
int main(void)
{
	
#ifdef UseADXL345
MyADXL345_Init(SPI1);
ID=MyADXL345_getID();
MyADXL345_getData(&User_g);
#endif	
	
#ifdef UseLTC2944	
  //************** Configuration LTC2944 par I2C *********************************************		
  // initialisation de la jauge
  UserLTC2944Conf.ADCmode=Control_ADCMode_Continuous;
  UserLTC2944Conf.ALCC=Control_ALCCConfigure_Disable;
  UserLTC2944Conf.PowerDown=Control_PowerDown_EnAnalogPart;
	LTC2944_Conf(I2C1,&UserLTC2944Conf);
	
	
  // Set Charge mAh
  Q=LTC2944_Set_SOC_mAh(17000.0);
#endif

#ifdef UseDS1307
	DS1307_Init(I2C1);
	// set date
	UserTimeSet.Year=22;
	UserTimeSet.Month=10;
	UserTimeSet.Date=13;
	UserTimeSet.Day=4;
	UserTimeSet.Hour=20;
	UserTimeSet.Min=54;
	UserTimeSet.Sec=0;
	
	//DS1307_SetTime(&UserTimeSet);
	DS1307_GetTime(&UserTimeSet);
#endif	
	
  while (1)
  {
		
#ifdef UseLTC2944		
		//************** read SOC ********************************************************
	  Q= LTC2944_Get_SOC_mAh();
	  //************** read Analog ********************************************************
 	  LTC2944_Get_AnalogVal(&LTC2944AnalogStruct);
	
		
		// delai
		for (tps=0;tps<100000;tps++)
		{
		}
#endif	  		
  
  }
 
}
