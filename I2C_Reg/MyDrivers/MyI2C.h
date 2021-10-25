#ifndef _I2C_
#define _I2C_



#include "stm32f10x.h"

//by periph team

/*
=====================     I2C les IO      ==============================================
deux p�riph�riques, I2C1 et I2C2
I2C1
SCL PB6
SDA PB7

I2C2
SCL PB10
SDA PB11

======= GRANDS PRINCIPES ================================================================

ATTENTION :
Emission et r�ception en mode ma�tre bloquant -> � lancer en background (ou dans une IT faible) 
pour pouvoir �tre interrompue par une autre interruption.

ATTENTION :
Une interruption est activ�e sur l'I2C consid�r�e. Elle met � jour la variable d'erreur.
L'erreur est accessible par la fct MyI2C_Err_Enum MyI2C_Get_Error(I2C_TypeDef * I2Cx)
BusError : Start et/ou stop conditions erron�es (mettre � 0 par soft)
AckFail  : l'ack n'est pas re�u (mettre � 0 par soft)
TimeOut	 : SCL est rest� plus de 25ms � l'�tat bas (mettre � 0 par soft)
UnknownError : pas de flag pr�cis...



Trame typique I2C en �criture plusieurs octets s'encha�nent :
|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   b7|b6|b5|b4|b3|b2|b1|b0|ACK|... |b7|b6|b5|b4|b3|b2|b1|b0|ACK|  Stop Cond| 

Trame typique I2C en lecture plusieurs octets (�criture pteur + lecture octets r�ception rb7..rb0 etc)
|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|ACK| 
Restart |rb7|rb6|rb5|rb4|rb3|rb2|rb1|rb0|ACK| ...|rb7|rb6|rb5|rb4|rb3|rb2|rb1|rb0|NACK|Stop Cond| 

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
	char Nb_Data;						// le nbre d'octets � envoyer / recevoir 
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

  */
void MyI2C_PutString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToSend);




/**
  * @brief  Ecrit juste un octet. Sera utilis� pour fixer l'@ pteur RTC juste avant une op�ration de lecture
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Addr7bits : @ I2C sur bo�tier RTC
  * @param  Octet : l'octet � envoyer
  * @retval None
  */
void MyI2C_PutChar(I2C_TypeDef * I2Cx, char Addr7bits, char Octet);





/*========================================================================================= 
														R�ception I2C : GetString 
========================================================================================= */

/**
	* @brief  Re�oit un paquet de donn�es : start, Slave@ Wr, datas, stop
	* le pointeur du bo�tier I2C est suppos� mis en place.
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress: @ du pointeur de registre interne du slave � partir duquel la 
	*         lecture commence.
  * @param  DataToReceive, structure qui contient les informations (cf.h)
	*		@param field 1 :SlaveAdress7bits, adresse du slave au format 7 bits.
	*										 NB : le code ins�re '1' au 8 �me bit pour indiquer la lecture
	*   @param field 2 :@ string � recevoir. C'est le programme appelant qui r�serve la table
	*										de r�ception.
	*   @param field 3 :Nbre de caract�re du string � recevoir
  * @retval None

Trame typique I2C en lecture :
	|Start Cond|Ad6|Ad5|Ad4|Ad3|Ad2|Ad1|Ad0|1 (=Read)|ACK|Octet 1|ACK|...|Octet n-1|ACK|Octet n|NACK|Stop Cond|
	|                 Address phase											|  Octet1  |...|  Octetn -1    | Octet n non acquitt�|  

NB : L'adresse I2C 7 bits est aquitt�e par le slave. Ensuite c'est l'inverse : le master aquitte car il re�oit 
les data (mais c'est lui qui g�n�re toujours les fronts puisqu'il est mastter).
Le master aquitte donc pour le slave sauf pour le dernier transfert : le Master 
n'acquitte pas. C'est signe pour le slave qu'il faut stopper.



*/
void	MyI2C_GetString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToReceive);




#endif
