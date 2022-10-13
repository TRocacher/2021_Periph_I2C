#ifndef _I2C_
#define _I2C_



#include "stm32f10x.h"

/*************************************************************************************
=====================    By Periph team 	INSA GEI		 ===============================
*************************************************************************************/

/*
*************************************************************************************
=====================     I2C les IO STM32F103      =================================
*************************************************************************************
I2C1
SCL PB6
SDA PB7

I2C2
SCL PB10
SDA PB11


*************************************************************************************
==================== Fondamentaux I2C      ==========================================
*************************************************************************************
- Bus synchrone Low speed (<100kHz) ou high speed (=400kHz), 
- Transfert octet par octet, poids fort en premier, avec aquittement pour chaque octet
- Deux lignes SDA et SCL (horloge) en open drain, repos '1'
- bit "normal" = SDA stable lors du pulse SCL (ie durant l'�tat haut de SCL SDA est stable)
- bit Start/Stop/Restart = SDA non stable lorsque SCL vaut '1'
			* Start : front descendant de SDA lorsque SCL vaut '1'
			* Stop  : front montant de SDA lorsque SCL = '1'
			* Restart = Start en cours de trame (typiquement pour changer Write/read).
- uC en Mode Master uniquement (c'est notre choix) : c'est le uC qui est ma�tre de l'horloge SCL.
- Le Slave a une @ 7 bits. On ajoute un bit 0 qui est /WR (donc 0 pour �criture, 1 pour lecture)
- Une adresse s'�crit donc a6 a5 a4 a3 a2 a1 a0 /WR ce qui donne bien 8 bits. Elle indique une future
lecture ou �criture. On parle tout de m�me d'@ 7 bits en regroupant a6 a5 a4 a3 a2 a1 a0

- On peut lire ou �crire une ou plusieurs donn�es � la suite. C'est lors de l'envoie de l'adresse Slave
du Master vers le slave que le sens � venir est indiqu�.
- En �criture, 
			* les Ack sont faits par le slave apr�s chaque octet envoy� par le master (Ack = mise � 0 le bit 9).
- En lecture, 
			* d�s que le l@ slave est transmise (/RW = 1), le slave positionne le bit 7 du prochain Byte � lire
			* le master encha�ne ses pulses (9), lors du pulse 9 (le dernier) c'est le master qui acquite.
			* Apr�s l'acquitement, le Slave amorce le prochain octet en positionnant son bit 7 sur SDA 
			* Apr�s le dernier octet, le Master g�n�re un stop.
					* Pour pouvoir g�n�rer le stop, le Master doit piloter SDA, or ce n'est pas possible puisque
					le Slave positionne le futur bit 7 ... Pour r�gler ce probl�me :
					lors du dernier transfert, le Master n'acquitte pas (NACK). Ainsi le Slave ne
					propose plus le bit 7 du prochain octet sur SDA. Le Master peut cl�turer la 
					communication avec un Stop.




======= Echange typique avec un Slave  ================================================================
- Une lecture ou �criture se fait vers un Slave et � partir d'une adresse m�moire donn�e (pointeur interne).
Ce pointeur est automatiquement incr�ment� lors des acc�s �criture ou lecture.

- Ecriture de N octets , trame compl�te (@ = adresse slave, pti = valeur de chargement du pointeur interne
|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |Slave ACK|
|pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|Slave ACK| 
|d7|d6|d5|d4|d3|d2|d1|d0|Slave ACK| (data 1)
.....
|d7|d6|d5|d4|d3|d2|d1|d0|Salve ACK|Stop Cond| (data N)

- Lecture de N octets � partir d'une adresse de pointeur donn�e
|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |Slave ACK|
|pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|Slave ACK| 
|ReStart Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =1 |Slave ACK|
|d7|d6|d5|d4|d3|d2|d1|d0|Master ACK| (data 1)
.....
|d7|d6|d5|d4|d3|d2|d1|d0|Master NACK|Stop Cond| (data N)





*************************************************************************************
==================== La lib I2C            ==========================================
*************************************************************************************






REMARQUE :
Une interruption est activ�e sur l'I2C consid�r�e (courte non bloquante). Elle met � jour la variable
d'erreur. L'erreur est accessible par la fct MyI2C_Err_Enum MyI2C_Get_Error(I2C_TypeDef * I2Cx)
BusError : Start et/ou stop conditions erron�es (mettre � 0 par soft)
AckFail  : l'ack n'est pas re�u (mettre � 0 par soft)
TimeOut	 : SCL est rest� plus de 25ms � l'�tat bas (mettre � 0 par soft)
UnknownError : pas de flag pr�cis...

NB : pour l'instant, la gestion d'erreur est incompl�te, il n'existe pas de timeout dans les fct
�criture et lecture... � am�liorer.


GRANDS PRINCIPE
Quelque soit le composant I2C, il est vu comme une m�moire que l'on peut �crire ou lire � partir
d'une adresse donn�e, qu'on appellerta PteurMem.

Pour une �criture ou lecture :
- on envoie l'adresse du composant I2C en attendant Ack.
- on envoie l'adresse PteurMem � partir de laquelle on veut lire la memmoire interne du composant
-> en �criture, on encha�ne autant d'octets qu'on veut qui s'inscrivent en suivant en RAM interne du
slave I2C
-> en lecture, on demande autant d'octets qu'on veut se situant � partir de l'adresse (PteurMem)
indiqu�e


DETAIL DES TRAMES ECRITURE

Trame typique I2C en �criture plusieurs octets s'encha�nent :
(�criture @slave I2C (@6..@0) + Wr + PteurMem pt7..pt0 + �criture octets  t7..t0 etc)
|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|ACK| 
|t7|t6|t5|t4|t3|t2|t1|t0|ACK| ...|t7|t6|t5|t4|t3|t2|t1|t0|ACK|Stop Cond|


Trame typique I2C en lecture plusieurs octets 
(�criture @slave I2C (@6..@0) + Wr + PteurMem pt7..pt0 + lecture octets r�ception r7..r0 etc)
NB : entre �criture et lecture, on ins�re une restart.
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
  * @param char IT_Prio_I2CErr 0 � 15 (utilis� en cas d'erreur, IT courte et non bloquante
  * @retval None
	
	
	DONNER EXEMPLE
  */
void MyI2C_Init(I2C_TypeDef * I2Cx, char IT_Prio_I2CErr);



/*========================================================================================= 
														EMISSION I2C : PutString
========================================================================================= */


// Structure de donn�es pour l'�mission ou la r�ception I2C :
typedef struct
{
	char SlaveAdress7bits;  // l'adresse I2C du slave device
	char * Ptr_Data;				// l'adresse du d�but de tableau char � recevoir/�mettre 
	char Nb_Data;						// le nbre d'octets � envoyer / recevoir  
}
MyI2C_RecSendData_Typedef;



/**
	* @brief|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |Slave ACK|
					|pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|Slave ACK| 
					|d7|d6|d5|d4|d3|d2|d1|d0|Slave ACK| (data 1)
					.....
					|d7|d6|d5|d4|d3|d2|d1|d0|Salve ACK|Stop Cond| (data N) 

  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress = adresse de d�marrage �criture � l'interieur du slave I2C
  * @param  DataToSend, adresse de la structure qui contient les informations � transmettre 
             voir d�finition	MyI2C_RecSendData_Typedef				
  * @retval None
  */
void MyI2C_PutString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToSend);










/*========================================================================================= 
														R�ception I2C : GetString 
========================================================================================= */

/**
	* @brief  |Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |Slave ACK|
						|pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|Slave ACK| 
						|ReStart Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =1 |Slave ACK|
						|d7|d6|d5|d4|d3|d2|d1|d0|Master ACK| (data 1)
						.....
						|d7|d6|d5|d4|d3|d2|d1|d0|Master NACK|Stop Cond| (data N)

  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress = adresse de d�marrage lecture � l'interieur du slave I2C
  * @param  DataToSend, adresse de la structure qui contient les informations n�cessaires � la
						r�ception des donn�es voir d�finition	MyI2C_RecSendData_Typedef				
  * @retval None
  */
void	MyI2C_GetString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToReceive);




#endif
