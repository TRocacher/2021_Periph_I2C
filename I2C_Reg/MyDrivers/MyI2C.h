#ifndef _I2C_
#define _I2C_


/*
*/

/*


========== I2C ==============================================
deux p�riph�riques, I2C1 et I2C2
I2C1
SCL PB6
SDA PB7

I2C2
SCL PB10
SDA PB11


*/


#include "stm32f10x.h"


/* ===== GRANDS PRINCIPES ================================================================

Emission et r�ception en mode ma�tre bloquant -> � lancer en background pour pouvoir
�tre interrompus par une autre interruption.

Interruption :
Non g�r�e sauf celles concernant les erreurs. Les types d'erreur sont :
BusError : Start et/ou stop conditions erron�es (mettre � 0 par soft)
AckFail  : l'ack n'est pas re�u (mettre � 0 par soft)
TimeOut	 : SCL est rest� plus de 25ms � l'�tat bas (mettre � 0 par soft)
UnknownError : pas de flag pr�cis...

Trame typique I2C en �criture :
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
	TimeOut,		// SCL est rest� plus de 25ms � l'�tat bas
	UnknownError // IT erreur d�clench�e mais pas de flag explicite ?
	
} MyI2C_Err_Enum;




/**
  * @brief  Retourne les erreurs I2C
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval Type d'erreur rencontr�e , voir MyI2C_Err_Enum
  */

MyI2C_Err_Enum MyI2C_Get_Error(I2C_TypeDef * I2Cx);



/*========================================================================================= 
														INITIALISATION I2C
========================================================================================= */





/**
  * @brief  Initialise l'interface I2C (1 ou 2) 
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param char IT_Prio_I2CErr 0 � 15

  * @retval None
  */
void MyI2C_Init(I2C_TypeDef * I2Cx, char IT_Prio_I2CErr);



/*========================================================================================= 
														EMISSION I2C : PutString
========================================================================================= */


// Structure de donn�e pour l'�mission ou la r�ception I2C :
typedef struct
{
	char SlaveAdress7bits;  // l'adresse I2C
	char * Ptr_Data;				// l'adresse de la ch�ine � envoyer / recevoir
	char Nb_Data;						// le nbre d'octets � envoyer / recevoir (gestion par le nbre, non par la d�tection de NULL)
}
MyI2C_RecSendData_Typedef;



/**
	* @brief  Envoie un paquet de donn�es : start, Slave I2C@, datas, stop
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  DataToSend, structure qui contient les informations 
	*		@param field 1 : SlaveAdress7bits, adresse du slave au format 7 bits.
	*										 NB : le code ins�re '0' au 8 �me bit pour indiquer l'�criture
	*   @param field 2 :@ string � �mettre.
	*   @param field 3 :Nbre de caract�re du string (le print ne travaille pas sur la recherche 
  *										de NULL mais sur le nbre d'octets, permettant d'envoyer n'importe quoi,
	*										y compris la valeur 0.
  * @retval None

	Trame typique I2C en �criture :
	|Start Cond|Ad6|Ad5|Ad4|Ad3|Ad2|Ad1|Ad0|0 (=Write)|ACK|Octet 1|ACK|...|Octet n|ACK|Stop Cond|
	|                 Address phase												|  Octet1   |...|  Octetn             |  

	L'adresse 7 bits est envoy�e avec un 0 formant le lsb (@8bit paire si on veut...). Le bit Ack
	est renvoy� par le slave (0).
	Ensuite les data 8 bits sont envoy�es en s�rie suivi d'un Ack du slave.
	Apr�s le dernier octet, stop condition conduisant au rel�chement ds lignes SDA et SCL.
  */
void MyI2C_PutString(I2C_TypeDef * I2Cx, MyI2C_RecSendData_Typedef * DataToSend);




/**
  * @brief  Ecrit juste un octet. Sera utilis� pour fixer l'@ pteur RTC juste avant une op�ration de lecture
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Addr7bits : @ I2C sur bo�tier RTC
  * @param  Octet : l'octet � envoyer
  * @retval None
  */
void MyI2C_PutChar(I2C_TypeDef * I2Cx, char Addr7bits, char Octet);





/*========================================================================================= 
														R�ception I2C : GetString !!! REPRENDRE le commentaire
========================================================================================= */

/**
	* @brief  Re�oit un paquet de donn�es : start, Slave@, datas, stop
	* le pointeur du bo�tier I2C est suppos� mis en place.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  DataToReceive, structure qui contient les informations (cf.h)
	*		@param field 1 : SlaveAdress7bits, adresse du slave au format 7 bits.
	*										 NB : le code ins�re '1' au 8 �me bit pour indiquer la lecture
	*   @param field 2 :@ string � recevoir.C'est le programme appelant qui r�serve la table
	*										de r�ception.
	*   @param field 3 :Nbre de caract�re du string � recevoir
  * @retval None

Trame typique I2C en lecture :
	|Start Cond|Ad6|Ad5|Ad4|Ad3|Ad2|Ad1|Ad0|1 (=Read)|ACK|Octet 1|ACK|...|Octet n-1|ACK|Octet n|NACK|Stop Cond|
	|                 Address phase											|  Octet1  |...|  Octetn -1    | Octet n non acquitt�|  

NB : L'adresse I2C 7 bits est aquitt�e par le slave. Ensuite c'est l'inverse : le master aquitte car il re�oit 
les data (mais c'est lui qui g�n�re toujours les fronts puisqu'il est mastter).
Le master aquitte donc pour le slave sauf pour le dernier transfert : le Master 
n'acquitte pas. C'est signe qu'il faut stopper.



*/
void	MyI2C_GetString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToRead);




#endif
