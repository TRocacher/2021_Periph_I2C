#ifndef _DRIVER_DS1307_H
#define _DRIVER_DS1307_H






#include "stm32f10x.h"
/* ====================================================================
	// Voir datasheet DS1307 et moodle, aide I2C "I2C mode d emploi pdf" page p6
	// @ 7bits = 0x68 1101000
	// ECRITURE : Adressage 0x68 (Write cad bit 0 = 0), puis écriture du pointeur d'adresse
	// puis écriture séquentielle.
	
	// LECTURE : en deux temps. D'abord une écriture I2C @0x68 (Write, cad bit 0 = 0) puis écr du pointeur
	// du composant. Puis nouveau start avec @I2C = 0x68 (read , cad bit 0 =1). Puis lecture séquentielle.
	

==========================================================================================*/

// Adresse I2C de la RTC DS1307
#define DS1307_Slave7bitsAdr 0x68

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
* @brief initialise l'I2C pour piloter le DS1307.

  * @param  le périph I2C à utiliser (100 kHz, Prio IT erreur =1, Own adress =0x55 -inutile-)
  * @retval None
*/
void DS1307_Init(void);



/**
* @brief permet juste de caler le pointeur avant d'enclencher une lecture série.
* il s'agit donc d'une opération Write suivi de l'adresse du pointeur :
* la trame  0x68 (Write) suivi de 0x00 (adresse pointeur de base RTC)

  * @param  la valeur de l'adresse du pointeur de registre du RTC (0 donc pour aller chercher les secondes
	* en premier)
  * @retval None
*/
void DS1307_SetPointer(char Pteur);



/**
* @brief  Mets à l'heure la RTC : doit envoyer 8 octets à partir de l'adresse 00h qui sont
*  1-Sec ;2-min ;3-Hours (format 12/24..) ; 4- Jour (Lundi - Dim) ; 5- date (1-31) ; 6- Mois; 
*  7-Année (diz et unit); 8-ctrl ( voir doc, 0 dans notre cas)
*  la trame I2C qui sera envoyée contient donc 9 octets : L'adresse pteur + 8 bytes
*  la trame est donc : 0x68 (Write) puis les 9 bytes 

  * @param  usertime (voir ci-dessus)
  * @retval None
*/
void DS1307_SetTime(DS1307_Time_Typedef * UserTime);




/**
* @brief  Récupère les 7 valeurs du RTC
* en deux temps , d'abord écriture pteur RTC
* puis lancement lecture séquentielle

  * @param  usertime (voir ci-dessus)
  * @retval None
*/
void DS1307_GetTime(DS1307_Time_Typedef * UserTime);



#endif
