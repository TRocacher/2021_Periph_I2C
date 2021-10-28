#ifndef _I2C_
#define _I2C_



#include "stm32f10x.h"

//by periph team

/*
=====================     I2C les IO      ==============================================
deux périphériques, I2C1 et I2C2
I2C1
SCL PB6
SDA PB7

I2C2
SCL PB10
SDA PB11

======= COMMENTAIRES ================================================================

ATTENTION :
Emission et réception en mode maître bloquant -> à lancer en background (ou dans une IT faible) 
pour pouvoir être interrompue par une autre interruption.

REMARQUE :
Une interruption est activée sur l'I2C considérée (courte non bloquante). Elle met à jour la variable
d'erreur. L'erreur est accessible par la fct MyI2C_Err_Enum MyI2C_Get_Error(I2C_TypeDef * I2Cx)
BusError : Start et/ou stop conditions erronées (mettre à 0 par soft)
AckFail  : l'ack n'est pas reçu (mettre à 0 par soft)
TimeOut	 : SCL est resté plus de 25ms à l'état bas (mettre à 0 par soft)
UnknownError : pas de flag précis...

NB : pour l'instant, la gestion d'erreur est incomplète, il n'existe pas de timeout dans les fct
écriture et lecture... à améliorer.


GRANDS PRINCIPE
Quelque soit le composant I2C, il est vu comme une mémoire que l'on peut écrire ou lire à partir
d'une adresse donnée, qu'on appellerta PteurMem.

Pour une écriture ou lecture :
- on envoie l'adresse du composant I2C en attendant Ack.
- on envoie l'adresse PteurMem à partir de laquelle on veut lire la memmoire interne du composant
-> en écriture, on enchaîne autant d'octets qu'on veut qui s'inscrivent en suivant en RAM interne du
slave I2C
-> en lecture, on demande autant d'octets qu'on veut se situant à partir de l'adresse (PteurMem)
indiquée


DETAIL DES TRAMES ECRITURE

Trame typique I2C en écriture plusieurs octets s'enchaînent :
(écriture @slave I2C (@6..@0) + Wr + PteurMem pt7..pt0 + écriture octets  t7..t0 etc)
|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|ACK| 
|t7|t6|t5|t4|t3|t2|t1|t0|ACK| ...|t7|t6|t5|t4|t3|t2|t1|t0|ACK|Stop Cond|


Trame typique I2C en lecture plusieurs octets 
(écriture @slave I2C (@6..@0) + Wr + PteurMem pt7..pt0 + lecture octets réception r7..r0 etc)
NB : entre écriture et lecture, on insère une restart.
|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|ACK| 
Restart |r7|r6|r5|r4|r3|r2|r1|r0|ACK| ...|r7|r6|r5|r4|r3|r2|r1|r0|NACK|Stop Cond| 

==========================================================================================*/



/*========================================================================================= 
														GESTION ERREURS 
========================================================================================= */
typedef enum 
{
	OK,
	BusError, 	// 
	AckFail,  	// Pas,d'ack
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
  * @param char IT_Prio_I2CErr 0 à 15 (utilisé en cas d'erreur, IT courte et non bloquante
  * @retval None
  */
void MyI2C_Init(I2C_TypeDef * I2Cx, char IT_Prio_I2CErr);



/*========================================================================================= 
														EMISSION I2C : PutString
========================================================================================= */


// Structure de données pour l'émission ou la réception I2C :
typedef struct
{
	char SlaveAdress7bits;  // l'adresse I2C
	char * Ptr_Data;				// l'adresse du début de tableau char à recevoir/émettre 
	char Nb_Data;						// le nbre d'octets à envoyer / recevoir 
}
MyI2C_RecSendData_Typedef;



/**
	* @brief  |Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|ACK| 
|t7|t6|t5|t4|t3|t2|t1|t0|ACK| ...|t7|t6|t5|t4|t3|t2|t1|t0|ACK|Stop Cond|

  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress = PteurMem, adresse de démarrage écriture à l'interieur du slave I2C
  * @param  DataToSend, adresse de la structure qui contient les informations à transmettre 
             voir définition	MyI2C_RecSendData_Typedef				
  * @retval None
  */
void MyI2C_PutString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToSend);










/*========================================================================================= 
														Réception I2C : GetString 
========================================================================================= */

/**
	* @brief  |Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|ACK| 
Restart |r7|r6|r5|r4|r3|r2|r1|r0|ACK| ...|r7|r6|r5|r4|r3|r2|r1|r0|NACK|Stop Cond| 

  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress = PteurMem, adresse de démarrage écriture à l'interieur du slave I2C
  * @param  DataToSend, adresse de la structure qui contient les informations nécessaires à la
						réception des données voir définition	MyI2C_RecSendData_Typedef				
  * @retval None
  */
void	MyI2C_GetString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToReceive);




#endif
