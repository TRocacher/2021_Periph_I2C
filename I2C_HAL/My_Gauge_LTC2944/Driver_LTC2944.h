/*
 * Driver_LTC2944.h
 *
 *  Created on: 16 oct. 2021
 *      Author: trocache
 */



/*
 *   DESCRIPTION DU CIRCUIT EN TERME DE PROG REG EN I2C...
 *
 *   Power on Reset (POR)
 *   Au passage de SENSE+ au delà de 3.3V
 *   - la jauge fonctionne
 *   - les mesures de tension, courant, temp sont off
 *   - les registres sont mis dans leur état pas défaut.
 *   - Le registre d'accumlation (jauge) est placé à mi échelle : 0x7FFF
 *
 *   Tous les registres sont des 8 bits.
 *
 *   Les mesures :
 *   - Voltage = tension sur SENSE-
 *   - Current = tension sur le shunt
 *
 *   Les registres intéressants (y en a beaucoup d'autres utlisés pour les alertes
 *   dont de seuils à régler divers et variés
 *   - A Status @0x0 : les alertes essentiellement
 *   - B Control @0x1 : réglage du circuit
 *
 *   - C Accumulated charge MSB @0x02,   Q[mAh] =
 *   - D Accumulated charge LSB @0x03
 *
 *   - I Voltage reg MSB @0x08  , V_SENSE-[V] = 70.8*Result (I-J) / 65535   (16bits res)
 *   - J Voltage reg LSB @0x09
 *
 *   - O Current Reg MSB @0x0E ,I[mA]=(0.064 / Rsense(Ohm)) . (Result (O-P) -32767)/32767 (12 bits, les 4 lsb = 0)
 *   - P Current Reg MSB @0x0F
 *
 *   - U Temp Reg  Reg MSB @0x14,    Temp[K] = 510K*Result / 65535 (11 bits, les 5 lsb = 0)
 *   - V Temp Reg  Reg MSB @0x15
 *
 *
 * 			CONSOMMATION
 * 	30 µA en mode shutdown
 * 	150 µA en mode coulomb counter ADC off
 * 	950 µA en mode coulomb counter ADC on


 */


#ifndef DRIVER_LTC2944_H_
#define DRIVER_LTC2944_H_


#include "stm32l4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
#define LTC2944_Slave7bitsAdr (0x64<<1)  //@slave I2C LTC2944

#define Rsense (0.006) //(0.012)
#define M (1024)

typedef struct
{
	char ADCmode; // see Control_ADCMode_.. values
	char ALCC;  //see Control_ALCCConfigure_.. values
	char PowerDown; //see Control_Control_PowerDown_.. values
}
LTC2944_Conf_Typedef;

/**
* @brief configure le LTC2944 au démarrage
  * @param  configuration structure pointer
  * @retval None
*/
void LTC2944_Conf(LTC2944_Conf_Typedef * Conf);



/**
* @brief Set the desired initial SOC in mAh
  * @param  SOC value in mAh
  * @retval Effective SOC value (regarding truncature effects)
*/
float LTC2944_Set_SOC_mAh(float SOC_mAh);




/**
* @brief get the actual SOC in the device
  * @param  none
  * @retval Effective SOC value
*/
float LTC2944_Get_SOC_mAh(void);



typedef struct
{
	float Voltage_V;
	float Current_mA;
	float Temperature_Celsius;
}
LTC2944_AnalogVal_Typedef;



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
void LTC2944_Get_AnalogVal(LTC2944_AnalogVal_Typedef * AnalogStructAddr);


/**
* @brief set the device in lowpower mode (no measure), analog part disable
  * @param  none
  * @retval none
*/
void LTC2944_Set_LowPowerMode(void);
// a faire

/**
* @brief set device in normal mode (analog part enable)
  * @param  none
  * @retval none
*/
void LTC2944_Set_AnalogOn(void);
// à faire

/**
* @brief stop analog part
  * @param  none
  * @retval none
*/
void LTC2944_Set_AnalogOff(void);
// à faire

//***************************************************************************************
//        REGISTER DEFINITIONS
//***************************************************************************************


// Control Reg Define
#define Control_ADCMode_Bits (0x3<<6)
#define Control_ADCMode_Continuous (0x3<<6)
#define Control_ADCMode_Scan10sec (0x2<<6)
#define Control_ADCMode_Manual (0x01<<6)
#define Control_ADCMode_Sleep (0x0<<6)  // reset Value

#define Control_PrescaleM_Bits (0x7<<3)
#define Control_PrescaleM_1 (0)
#define Control_PrescaleM_4 (0x1<<3)
#define Control_PrescaleM_16 (0x2<<3)
#define Control_PrescaleM_64 (0x3<<3)
#define Control_PrescaleM_256 (0x4<<3)
#define Control_PrescaleM_1024 (0x5<<3)
#define Control_PrescaleM_4096 (0x6<<3)
#define Control_PrescaleM_Default_4096 (0x7<<3) // reset Value

#define Control_ALCCConfigure_Bits (0x7<<3)
#define Control_ALCCConfigure_AlertMode (0x2<<3)  // reset Value
#define Control_ALCCConfigure_ChargeCompleteMode (0x1<<3)
#define Control_ALCCConfigure_Disable (0x0<<3)
// (0x3<<3) // forbidden conf !!

#define Control_PowerDown_Bits (0x1<<0)
#define Control_PowerDown_ShdAnalogPart (0x1<<0)  // descend la conso à 15µA, I2C OK, Reg retained, no measure
#define Control_PowerDown_EnAnalogPart (0x0<<0) // reset Value



// Registers @
#define B_Control 0x01
#define C_AccumulateChargeMSB (0x02)
#define D_AccumulateChargeLSB (0x03)
#define I_Voltage_MSB (0x08)
#define J_Voltage_LSB (0x09)
#define O_Current_MSB (0x0E)
#define P_Current_MSB (0x0F)
#define U_Temperature_MSB (0x14)
#define V_Temperature_LSB (0x15)


//***************************************************************************************
//        APP NOTE : How to choose Rsense, M ?
//***************************************************************************************

/*
 *  Rsense = 50mV / Imax. Ici, on prendra 3A.
 *  Rsense = 50m/3 = 16.6mOhm, on choisit 15mOhm (I max = 3.33A)
 *
 *  qLSB = 0.340mAh * (50m/Rsense).M/4096
 *  Soit Qmax la charge maximale de la batterie (par exemple 20Ah).
 *  Soit N le compteur C-D (accumulateChargeMESB & LSB)
 *  Nmax = Qmax / qLSB = Qmax*4096/(M*0.340mAh * (50m/Rsense))
 *
 *  ce qui donne en fonction des éléments fixés :
 *  Nmax = 20000*4096/(M*0.340mAh * (50m/15m)) = 72.28e6 /M.
 *  Prenons M = 4096
 *  NMax = 17647 < 65536 donc OK.
 *
 *
 *
 *
 *
 *  Dans cette configuration Rsense = 15mOhm, M=4096
 *  - la précision de mesure de courant est la plus grande car Rsense max
 *  - l'utilisation de la jauge est discutable, on a moins de 1/4 de la PE utilisée
 *  - la résolution du SOC est :  qLSB = 0.340mAh * (50m/15m).4096/4096 = 1.13mAh.
 *
 *
 *  Essayons avec M = 1024 , Rsense = 15mOhm, Cmax = 20Ah
 *  - la précision de mesure de courant est la plus grande car Rsense max
 *  - la résolution du SOC est :  qLSB = 0.340mAh * (50m/15m).1024/4096 = 0.283mAh.
 *  - la valeur maximale comptée est Qmax / Qlsb = 20000/0.283 = 70500 > 65536
 *  - inversement, Qmax = Qlsb*65536 =18.5Ah.
 *  - La batterie étant de 17Ah, ça passe.
 *
 *  ATTENTION, la batterie s'usant il se peut que la jauge doive indiquer +20% max ?
 *  Donc si 17Ah, prévoir 20.5 Ah.
 *  Si l'on souhaite rester avec M=1024, il faut diminuer un peu Rsense.
 *
 *
 *
 *  Prenons Rsense = 12mOhm, M=1024
 *   - la résolution du SOC est :  qLSB = 0.340mAh * (50m/12m).1024/4096 = 0.354mAh.
 *   - Qmax = Qlsb*65536 =0.354 * 65536 = 23Ah.
 *   - on a un peu perdu en résolution de mesure courant :
 *   	Imax = 50m/12m = 4.16A
 *   	Ilsb = 4.16 / 2^12 (12 bits sur le courant) = 1.017mA.
 *
 *  Cela semble être un bon compromis...
 */




#endif /* DRIVER_LTC2944_H_ */
