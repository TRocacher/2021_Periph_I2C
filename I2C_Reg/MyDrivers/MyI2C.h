#ifndef _I2C_
#define _I2C_


/*
*/

/*


========== I2C ==============================================
deux périphériques, I2C1 et I2C2
I2C1
SCL PB6
SDA PB7

I2C2
SCL PB10
SDA PB11


*/


#include "stm32f10x.h"


/* ===== GRANDS PRINCIPES ================================================================

Emission et réception en mode maître bloquant -> à lancer en background pour pouvoir
être interrompus par une autre interruption.

Interruption :
Non gérée sauf celles concernant les erreurs. Les types d'erreur sont :
BusError : Start et/ou stop conditions erronées (mettre à 0 par soft)
AckFail  : l'ack n'est pas reçu (mettre à 0 par soft)
TimeOut	 : SCL est resté plus de 25ms à l'état bas (mettre à 0 par soft)
UnknownError : pas de flag précis...

Trame typique I2C en écriture :
|Start Cond|b7|b6|b5|b4|b3|b2|b1|b0|ACK|Stop Cond| 

==========================================================================================*/


/*========================================================================================= 
														GESTION ERREURS 
========================================================================================= */
typedef enum 
{
	OK,
	BusError, 	// 
	AckFail,  	// 
	TimeOut,		// SCL est resté plus de 25ms à l'état bas
	UnknownError // IT erreur déclenchée mais pas de flag explicite ?
	
} MyI2C_Err_Enum;




/**
  * @brief  Retourne les erreurs I2C
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval Type d'erreur rencontrée , voir MyI2C_Err_Enum
  */

MyI2C_Err_Enum MyI2C_Get_Error(I2C_TypeDef * I2Cx);



/*========================================================================================= 
														INITIALISATION I2C
========================================================================================= */





/**
  * @brief  Initialise l'interface I2C (1 ou 2) 
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param char IT_Prio_I2CErr 0 à 15

  * @retval None
  */
void MyI2C_Init(I2C_TypeDef * I2Cx, char IT_Prio_I2CErr);



/*========================================================================================= 
														EMISSION I2C : PutString
========================================================================================= */


// Structure de donnée pour l'émission ou la réception I2C :
typedef struct
{
	char SlaveAdress7bits;  // l'adresse I2C
	char * Ptr_Data;				// l'adresse de la châine à envoyer / recevoir
	char Nb_Data;						// le nbre d'octets à envoyer / recevoir (gestion par le nbre, non par la détection de NULL)
}
MyI2C_RecSendData_Typedef;



/**
	* @brief  Envoie un paquet de données : start, Slave I2C@, datas, stop
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  DataToSend, structure qui contient les informations 
	*		@param field 1 : SlaveAdress7bits, adresse du slave au format 7 bits.
	*										 NB : le code insère '0' au 8 ème bit pour indiquer l'écriture
	*   @param field 2 :@ string à émettre.
	*   @param field 3 :Nbre de caractère du string (le print ne travaille pas sur la recherche 
  *										de NULL mais sur le nbre d'octets, permettant d'envoyer n'importe quoi,
	*										y compris la valeur 0.
  * @retval None

	Trame typique I2C en écriture :
	|Start Cond|Ad6|Ad5|Ad4|Ad3|Ad2|Ad1|Ad0|0 (=Write)|ACK|Octet 1|ACK|...|Octet n|ACK|Stop Cond|
	|                 Address phase												|  Octet1   |...|  Octetn             |  

	L'adresse 7 bits est envoyée avec un 0 formant le lsb (@8bit paire si on veut...). Le bit Ack
	est renvoyé par le slave (0).
	Ensuite les data 8 bits sont envoyées en série suivi d'un Ack du slave.
	Après le dernier octet, stop condition conduisant au relâchement ds lignes SDA et SCL.
  */
void MyI2C_PutString(I2C_TypeDef * I2Cx, MyI2C_RecSendData_Typedef * DataToSend);




/**
  * @brief  Ecrit juste un octet. Sera utilisé pour fixer l'@ pteur RTC juste avant une opération de lecture
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Addr7bits : @ I2C sur boîtier RTC
  * @param  Octet : l'octet à envoyer
  * @retval None
  */
void MyI2C_PutChar(I2C_TypeDef * I2Cx, char Addr7bits, char Octet);





/*========================================================================================= 
														Réception I2C : GetString !!! REPRENDRE le commentaire
========================================================================================= */

/**
	* @brief  Reçoit un paquet de données : start, Slave@, datas, stop
	* le pointeur du boîtier I2C est supposé mis en place.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  DataToReceive, structure qui contient les informations (cf.h)
	*		@param field 1 : SlaveAdress7bits, adresse du slave au format 7 bits.
	*										 NB : le code insère '1' au 8 ème bit pour indiquer la lecture
	*   @param field 2 :@ string à recevoir.C'est le programme appelant qui réserve la table
	*										de réception.
	*   @param field 3 :Nbre de caractère du string à recevoir
  * @retval None

Trame typique I2C en lecture :
	|Start Cond|Ad6|Ad5|Ad4|Ad3|Ad2|Ad1|Ad0|1 (=Read)|ACK|Octet 1|ACK|...|Octet n-1|ACK|Octet n|NACK|Stop Cond|
	|                 Address phase											|  Octet1  |...|  Octetn -1    | Octet n non acquitté|  

NB : L'adresse I2C 7 bits est aquittée par le slave. Ensuite c'est l'inverse : le master aquitte car il reçoit 
les data (mais c'est lui qui génère toujours les fronts puisqu'il est mastter).
Le master aquitte donc pour le slave sauf pour le dernier transfert : le Master 
n'acquitte pas. C'est signe qu'il faut stopper.



*/
void	MyI2C_GetString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToRead);




#endif
