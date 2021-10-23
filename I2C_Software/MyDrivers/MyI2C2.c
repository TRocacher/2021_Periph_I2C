#include "MyI2C2.h"


/* ===== GRANDS PRINCIPES ================================================================

IT timer au 1/4 de période SCL pour décomposer SDA/SCL

Plusieurs fcts différentes vont être utilisées. Chacune d'elle va :
- initialiser un ptr de fonction
- autoriser les IT au niveau timer.
Lorsqu'on rentre dans l'IT :
- acquittement IT
- Lancement ptr de fonction

Les fonctions sont construites sur le même principe :
- un compteur modulo gère les phases du séquencement

Fonctions :
MyI2C_SendByteWithStart(byte) : envoie un octet avec mise en place du start.
Renvoie Ack.



==========================================================================================*/

/******************************************************************************
*  PRIVATE Variables & fct
*
******************************************************************************/
static MyTimer_Struct_TypeDef MyTimer;
static MyGPIO_Struct_TypeDef MyGPIO;

char CharToSent;
int AckValue;
static char ErrorAck;


//variable type énumérée pour décrire les états de transmission I2C
typedef enum {
	Init,
	Start,
	Stop,
	Octet_AckFromSlave,
	Octet_AckFromMaster, 
	Octet_NackFromMaster,
	ReadAck,
	Idle}
 I2C_Transm_State;

I2C_Transm_State I2C_State;



/******************************************************************************
*  PUBLIC FUNCTION

******************************************************************************/
/**
  * @brief 
  */
void MyI2C_Init(void)
{
	// config IO PB6 et PB7 en output open drain
	MyGPIO.GPIO=I2C_SCL_GPIO;
	MyGPIO.GPIO_Pin=I2C_SCL_Pin;
	MyGPIO.GPIO_Conf=Out_Ppull ; //////////////OD
	MyGPIO_Init(&MyGPIO);
	MyGPIO.GPIO_Pin=I2C_SDA_Pin;
	MyGPIO_Init(&MyGPIO);
	
	
	// Conf IT à 400kHz (4 IT/période)
	MyTimer.ARR=I2C_TimerARR;
	MyTimer.PSC=I2C_TimerPSC;
	MyTimer.Timer=I2C_Timer;
	MyTimer_Base_Init(&MyTimer);
	
	// conf IT
  __NVIC_EnableIRQ(TIM4_IRQn);	
	__NVIC_SetPriority(TIM4_IRQn, I2C_TimerPrio);
	MyTimer_Base_Start(I2C_Timer);
	// au niveau timer
	I2C_Timer->DIER|=TIM_DIER_UIE;
	
	AckValue=0;
	I2C_State=Init;
	ErrorAck=0;
	while(I2C_State!=Idle){}
}


/*
Principe :

Plusieurs type de transmission (décomposition de trame I2C) peuvent être envisagées, toutes exclusives
- Init
- Start
- Stop
- 8Bits_AckFromSlave (master write)
- 8bits_AckFromMaster (master Read)
- 8bits_NackFromMaster (master Read last)
- Idle

Ces divers type de transmission sont un type énuméré I2C_MssgType.

Chaque grande fonction (putchar, putstring ...) devra positionner la variable énumérée. 
La valeur Idle ne fait rien. Elle est positionnée par la routine d'IT lorsque chaque tâche est terminée pour informer
la fonction appelante d'évoluer jusqu'à la prochaine étape.
Init positionne SDA et SCL à '1'


*/


char MyI2C_PutChar(char Addr7bits, char Octet)
{
	char Error=0;
	CharToSent=Addr7bits<<1;
	I2C_State=Start;
	while(I2C_State!=Idle){}
	I2C_State=Octet_AckFromSlave;
	while(I2C_State!=Idle){}
  if 	(AckValue==0) // pas d'erreur, on enchaîne avec l'octet à transmettre 
	{
			CharToSent=Octet;
			I2C_State=Octet_AckFromSlave;
			while(I2C_State!=Idle){} 
			if 	(AckValue!=0) 
			{
				Error=1;
			}
			
	}
	else Error=1;
	I2C_State=Stop;
	while(I2C_State!=Idle){}
		
	return Error;	
}






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
char MyI2C_PutString( MyI2C_RecSendData_Typedef * DataToSend)

{
	char * Ptr_Data;
	int Cpt_Byte;
	Ptr_Data=DataToSend->Ptr_Data;
	Cpt_Byte=DataToSend->Nb_Data;

	//==============START + Slave Adr ==================================================

	// Start order
	I2C_State=Start;
	while(I2C_State!=Idle){}
	
  // Slave Adress Emission
	CharToSent=(DataToSend->SlaveAdress7bits)<<1;
	I2C_State=Octet_AckFromSlave;
	while(I2C_State!=Idle){}	
		

//  //============== Fin phase Addressage ==================================================

//	//============== emission de N-1 premiers octets =======================================

	while (Cpt_Byte>1)
	{
	  CharToSent=*Ptr_Data;
		I2C_State=Octet_AckFromSlave;
		while(I2C_State!=Idle){}
		Cpt_Byte--;
		Ptr_Data++;
		
	}
	
	//============== STOP condition ======================================================
	I2C_State=Stop;
	while(I2C_State!=Idle){}
	if (ErrorAck==1) 
	{
		ErrorAck=0;
		return 0;
	}
	else return 1;
}















//*****************************************************
//*********** IT TIM pour I2C *************************
int Cnt=0;
char BitMsk;

void TIM4_IRQHandler(void)
{
	TIM4->SR&=~TIM_SR_UIF;
	
	// Modulo cnt
	Cnt++;
	if (Cnt==4) Cnt=0;
	
	switch(I2C_State)
	{
		case Init:
		{
			if (Cnt==0)
			{
				MyGPIO_Set(I2C_SCL_GPIO, I2C_SCL_Pin);
				MyGPIO_Set(I2C_SDA_GPIO, I2C_SDA_Pin);
				I2C_State=Idle;
			}
			break;
		}
		
		case Start:
		{
			if (Cnt==0)	MyGPIO_Reset(I2C_SDA_GPIO, I2C_SDA_Pin);
			else if (Cnt==2)	
			{
				MyGPIO_Reset(I2C_SCL_GPIO, I2C_SCL_Pin);
				I2C_State=Idle;
				BitMsk=1<<7;
			}
			break;
		}
		
		case Stop:
		{
			if (Cnt==0)	MyGPIO_Set(I2C_SCL_GPIO, I2C_SCL_Pin);
			else if (Cnt==2)	
				{
				MyGPIO_Set(I2C_SDA_GPIO, I2C_SDA_Pin);
				I2C_State=Idle;
				}

			break;
		}
		
		
		case Octet_AckFromSlave:
		{
			if (Cnt==0)	// positionnement bit SDA
			{
				// test du bit courant, le sortir sur SDA
				if ((CharToSent&BitMsk)==BitMsk) MyGPIO_Set(I2C_SDA_GPIO, I2C_SDA_Pin);
				else MyGPIO_Reset(I2C_SDA_GPIO, I2C_SDA_Pin);
				BitMsk=BitMsk>>1;
			}
			else if (Cnt==1) MyGPIO_Set(I2C_SCL_GPIO, I2C_SCL_Pin);	 // front montant SCL	
			else if (Cnt==2) MyGPIO_Reset(I2C_SCL_GPIO, I2C_SCL_Pin);	 // front desc SCL	
			else if (Cnt==3) 
			{
				if (BitMsk==0) 
				{ 
					I2C_State=ReadAck;// les 8 bits sont passés, 
					MyGPIO_Set(I2C_SDA_GPIO, I2C_SDA_Pin); // on libère SDA pour une réponse du slave
					BitMsk=1<<7; // prepa next octet
				}
			}
			break;
		}
		
		
		case ReadAck:
		{
			if (Cnt==1)	// front up SCL et lecture Ack
			{
				MyGPIO_Set(I2C_SCL_GPIO, I2C_SCL_Pin);	 // front montant SCL	
				AckValue = MyGPIO_Read(I2C_SDA_GPIO, I2C_SDA_Pin); // si AckValue vaut 0, OK sinon soucis !! le slave n'a pas acquité
			}
			else if (Cnt==2)
			{
				 MyGPIO_Reset(I2C_SCL_GPIO, I2C_SCL_Pin);	 // front desc SCL		
				 I2C_State=Idle;
			}				
			break;	
			if 	(AckValue!=0) ErrorAck=1; // mise à 1 si pb, remis à 0 ailleurs...
		}


		
		case Octet_AckFromMaster:break;
		case Octet_NackFromMaster:break;
		case Idle:break;
		default:break;
	}
	
	
	
}
