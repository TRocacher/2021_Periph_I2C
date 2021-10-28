#include "Driver_DS1307.h"
#include "stm32f10x.h"
#include "Driver_LTC2944.h"


//************** Var RTC DS1307 *******************************************************
DS1307_Time_Typedef UserTimeSet;
//************** Var Gauge LTC2944 *******************************************************
LTC2944_Conf_Typedef UserLTC2944Conf;
LTC2944_AnalogVal_Typedef LTC2944AnalogStruct;
float Q;


int tps;
int main(void)
{
	
  //************** Configuration LTC2944 par I2C *********************************************		
  // initialisation de la jauge
  UserLTC2944Conf.ADCmode=Control_ADCMode_Continuous;
  UserLTC2944Conf.ALCC=Control_ALCCConfigure_Disable;
  UserLTC2944Conf.PowerDown=Control_PowerDown_EnAnalogPart;
	LTC2944_Conf(I2C1,&UserLTC2944Conf);
	
	
  // Set Charge mAh
  Q=LTC2944_Set_SOC_mAh(17000.0);

	
	
  while (1)
  {
		
	
		//************** read SOC ********************************************************
	  Q= LTC2944_Get_SOC_mAh();
	  //************** read Analog ********************************************************
 	  LTC2944_Get_AnalogVal(&LTC2944AnalogStruct);
	
		
		// delai
		for (tps=0;tps<100000;tps++)
		{
		}
	  		
  
  }
 
}
