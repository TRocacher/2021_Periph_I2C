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

/*
I2C2
Utilise un timer en IT

*/



#include "stm32f10x.h"
#include "MyTimer.h"
#include "MyGPIO.h"

/* ===== GRANDS PRINCIPES ================================================================

Emission et r�ception en mode ma�tre bloquant -> � lancer en background pour pouvoir
�tre interrompus par une autre interruption.



==========================================================================================*/



/******************************************************************************
*  Registers Define
******************************************************************************/
#define I2C_Timer TIM4
#define I2C_SCL_Pin 6
#define I2C_SCL_GPIO GPIOB
#define I2C_SDA_Pin 7
#define I2C_SDA_GPIO GPIOB

/* fr�quence SCL 100kHz. Le programme s�pare en 4 tps 
typiquement (hors start/strop)
- nvelle SDA, SCL = 0
- front up SCL
- front bas SCL
- attente
PSC=0, ARR = 72 M / 100k /4 = 180.
Dur�e 1/4 p�riode = 2.5�s...
*/
#define I2C_TimerARR (180-1)
#define I2C_TimerPSC (0)
#define I2C_TimerPrio (0)   // priorit� maximale pour pas �tre d�rang� pendant un transfert I2C.


/******************************************************************************
*  Public Functions
******************************************************************************/

// Structure de donn�e pour l'�mission ou la r�ception I2C :
typedef struct
{
	char SlaveAdress7bits;  // l'adresse I2C
	char * Ptr_Data;				// l'adresse de la ch�ine � envoyer / recevoir
	char Nb_Data;						// le nbre d'octets � envoyer / recevoir (gestion par le nbre, non par la d�tection de NULL)
}
MyI2C_RecSendData_Typedef;




/**
  * @brief 
  */
void MyI2C_Init(void);




/**
  * @brief  Ecrit juste un octet. Sera utilis� pour fixer l'@ pteur RTC juste avant une op�ration de lecture
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  Addr7bits : @ I2C sur bo�tier RTC
  * @param  Octet : l'octet � envoyer
  * @retval None
  */
char MyI2C_PutChar(char Addr7bits, char Octet);




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
  * @retval 0 si erreur

	Trame typique I2C en �criture :
	|Start Cond|Ad6|Ad5|Ad4|Ad3|Ad2|Ad1|Ad0|0 (=Write)|ACK|Octet 1|ACK|...|Octet n|ACK|Stop Cond|
	|                 Address phase												|  Octet1   |...|  Octetn             |  

	L'adresse 7 bits est envoy�e avec un 0 formant le lsb (@8bit paire si on veut...). Le bit Ack
	est renvoy� par le slave (0).
	Ensuite les data 8 bits sont envoy�es en s�rie suivi d'un Ack du slave.
	Apr�s le dernier octet, stop condition conduisant au rel�chement ds lignes SDA et SCL.
  */
char MyI2C_PutString(MyI2C_RecSendData_Typedef * DataToSend);



/**
  * @brief bof.....
  */
char MyI2C_SendByteWithStart(char byte);




#endif
