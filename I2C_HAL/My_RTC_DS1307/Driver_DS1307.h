#ifndef _DRIVER_DS1307_H
#define _DRIVER_DS1307_H







#include "stm32l4xx_hal.h"

/* ====================================================================
	// Voir datasheet DS1307 et moodle, aide I2C "I2C mode d emploi pdf" page p6
	// @ 7bits = 0x68 1101000
	// ECRITURE : Adressage 0x68 (Write cad bit 0 = 0), puis �criture du pointeur d'adresse
	// puis �criture s�quentielle.
	
	// LECTURE : en deux temps. D'abord une �criture I2C @0x68 (Write, cad bit 0 = 0) puis �cr du pointeur
	// du composant. Puis nouveau start avec @I2C = 0x68 (read , cad bit 0 =1). Puis lecture s�quentielle.
	

==========================================================================================*/


// i2C associé (handle déclaré dans le main par cubemx)
extern I2C_HandleTypeDef hi2c1;

// Adresse I2C de la RTC DS1307
#define DS1307_Slave7bitsAdr (0x68<<1)


typedef struct
{
	char Sec;
	char Min;
	char H_12_Not_24;
	char PM_Not_AM; // 1 PM, 0 AM
	char Hour;
	char Day;
	char Date;
	char Month;
	char Year;
}
DS1307_Time_Typedef;






/**
* @brief permet juste de caler le pointeur avant d'enclencher une lecture s�rie.
* il s'agit donc d'une op�ration Write suivi de l'adresse du pointeur :
* la trame  0x68 (Write) suivi de 0x00 (adresse pointeur de base RTC)

  * @param  la valeur de l'adresse du pointeur de registre du RTC (0 donc pour aller chercher les secondes
	* en premier)
  * @retval None
*/
void DS1307_SetPointer(uint8_t Pteur);



/**
* @brief  Mets � l'heure la RTC : doit envoyer 8 octets � partir de l'adresse 00h qui sont
*  1-Sec ;2-min ;3-Hours (format 12/24..) ; 4- Jour (Lundi - Dim) ; 5- date (1-31) ; 6- Mois; 
*  7-Ann�e (diz et unit); 8-ctrl ( voir doc, 0 dans notre cas)
*  la trame I2C qui sera envoy�e contient donc 9 octets : L'adresse pteur + 8 bytes
*  la trame est donc : 0x68 (Write) puis les 9 bytes 

  * @param  usertime (voir ci-dessus)
  * @retval None
*/
void DS1307_SetTime(DS1307_Time_Typedef * UserTime);




/**
* @brief  R�cup�re les 7 valeurs du RTC
* en deux temps , d'abord �criture pteur RTC
* puis lancement lecture s�quentielle

  * @param  usertime (voir ci-dessus)
  * @retval None
*/
void DS1307_GetTime(DS1307_Time_Typedef * UserTime);



#endif
