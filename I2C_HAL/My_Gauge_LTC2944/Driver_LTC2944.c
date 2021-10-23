#include "Driver_LTC2944.h"

/**
* @brief configure le LTC2944 au démarrage
  * @param  configuration structure pointer
  * @retval None
*/


static float qLSB_mAh; // le calcul est fait dans l'init et réutilisé par la suite
static float iLSB_mA;
static float vLSB_V;
static float tLSB_Kelv;
static uint8_t data[15];



void LTC2944_Conf(LTC2944_Conf_Typedef * Conf)
{
    // data[2] = @B_Control ; Control val
	uint8_t ConfReg;
	ConfReg=0;
	ConfReg|=Conf->ADCmode;
	ConfReg|=Conf->ALCC;
	ConfReg|=Conf->PowerDown;
	switch (M)
	{
	case 4096: ConfReg|=Control_PrescaleM_4096;break;
	case 1024: ConfReg|=Control_PrescaleM_1024;break;
	case 256: ConfReg|=Control_PrescaleM_256;break;
	case 64: ConfReg|=Control_PrescaleM_64;break;
	case 16: ConfReg|=Control_PrescaleM_16;break;
	case 4: ConfReg|=Control_PrescaleM_4;break;
	case 1: ConfReg|=Control_PrescaleM_1;break;
	default: while(1); // mauvais prescaler, n'existe pas dans la liste !
	}
	data[0]=B_Control;
	data[1]=ConfReg;
	HAL_I2C_Master_Transmit(&hi2c1, LTC2944_Slave7bitsAdr, data,2, HAL_MAX_DELAY);
	//qLSB = 0.340mAh * (50m/Rsense).M/4096
	qLSB_mAh=0.34* (0.05/Rsense)*(float)M/4096.0;

	//I[mA]=(0.064 / Rsense(Ohm)) . (Result (O-P) -32767)/32767 (12 bits, les 4 lsb = 0)
	iLSB_mA=(0.064/Rsense)/32767.0;

	//V_SENSE-[V] = 70.8*Result (I-J) / 65535   (16bits res)
	vLSB_V=70.8/65535.0;

	//Temp[K] = 510K*Result / 65535 (11 bits, les 5 lsb = 0)
	tLSB_Kelv=510.0/65535.0;
}



/**
* @brief Set the desired initial SOC in mAh
  * @param  SOC value in mAh
  * @retval Effective SOC value (regarding truncature effects)
*/
float LTC2944_Set_SOC_mAh(float SOC_mAh)
{
	int N;
    // data[3] = @ C_AccumulateChargeMSB ; MSB ; LSB
	N=SOC_mAh/qLSB_mAh;
	data[0]=C_AccumulateChargeMSB;
	data[1]=N>>8; // MSB
	data[2]=N&0x00FF; // LSB
	HAL_I2C_Master_Transmit(&hi2c1, LTC2944_Slave7bitsAdr, data,3, HAL_MAX_DELAY);
    return ((float)N*qLSB_mAh);
}


/**
* @brief get the actual SOC in the device
  * @param  none
  * @retval Effective SOC value
*/
float LTC2944_Get_SOC_mAh(void)
{
	int N;
	data[0]=0;
	data[1]=0;
	// Set adress reg
	HAL_I2C_Mem_Read(&hi2c1, LTC2944_Slave7bitsAdr, C_AccumulateChargeMSB,1,data,2,HAL_MAX_DELAY);
	N=data[1]+256*data[0];
    return ((float)N*qLSB_mAh);
}



/**
* @brief get the actual analog values
* La fonction retourne l'ensemble des 3 valeurs analogiques.
* Le LTC est supposé avoir été lancé auparavant.
* La conversion dure approximativement 33ms pour le voltage 4.5ms pour le courant
* et la température.
* Soit un lancement est opéré toutes les 50ms (pour être tranquille) puis le résultat est mesuré
* Soit on met le circuit en mode automatique continue.
* Il n'y  a pas de flag de fin de conversion.

  * @param  none
  * @retval Structure adress
*/
void LTC2944_Get_AnalogVal(LTC2944_AnalogVal_Typedef * AnalogStructAddr)
{
	int N;
	float temp_Celsius;
	data[0]=0;
	data[1]=0;
    // get Voltage
	HAL_I2C_Mem_Read(&hi2c1, LTC2944_Slave7bitsAdr, I_Voltage_MSB,1,data,2,HAL_MAX_DELAY);
	N=data[1]+256*data[0];
	AnalogStructAddr->Voltage_V=(float)N*vLSB_V;

	// get current
	HAL_I2C_Mem_Read(&hi2c1, LTC2944_Slave7bitsAdr, O_Current_MSB,1,data,2,HAL_MAX_DELAY);
	N=data[1]+256*data[0];
	N=N-32767; // N est signé maintenant
	AnalogStructAddr->Current_mA=(float)N*iLSB_mA;

	//get temperature
	HAL_I2C_Mem_Read(&hi2c1, LTC2944_Slave7bitsAdr, U_Temperature_MSB,1,data,2,HAL_MAX_DELAY);
	N=data[1]+256*data[0];
	temp_Celsius=(float)N*tLSB_Kelv-273.15;
	AnalogStructAddr->Temperature_Celsius=temp_Celsius;


}

