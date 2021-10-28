#include "MyI2C.h"


/* ======================================================================================

//by periph team

Emission et réception en mode maître bloquant -> à lancer en background pour pouvoir
être interrompus par une autre interruption.

==========================================================================================*/

// il se peut que le périph plante (flag busy). Décommenter le define ci-dessous pour contourner
// le problème. 
//#define FixBugBusyFlag


//***************************************************************************************
//=======================================================================================
//          DEBUT : Inclusion fcts GPIO perso pour que le module soit autonome
//=======================================================================================
//***************************************************************************************

typedef struct
{
	GPIO_TypeDef * GPIO;
	char GPIO_Pin;    // numéro de pin 0 à 15
	char GPIO_Conf;   // see below
} MyGPIO_Struct_TypeDef;

#define In_Floating   0x4  //   0100
#define In_PullDown		0x8  //   1000 pdwn
#define In_PullUp     0x9  //   1001 !!! code spécial pour pull up
#define In_Analog		  0    //   0000 
#define Out_Ppull 	  0x2  //   0010
#define Out_OD        0x6  //   0110
#define AltOut_Ppull  0xA  //   1010
#define AltOut_OD			0xE  //   1110

// CNF MOD
// xx xx 

void MyGPIO_Init( MyGPIO_Struct_TypeDef *  GPIOStructPtr)
{

	if (GPIOStructPtr->GPIO==GPIOA) RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
	else if (GPIOStructPtr->GPIO==GPIOB) RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
	else if (GPIOStructPtr->GPIO==GPIOC) RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;
	else if (GPIOStructPtr->GPIO==GPIOD) RCC->APB2ENR|=RCC_APB2ENR_IOPDEN;
	else while(1);// port n'existe pas
	
	if ( GPIOStructPtr->GPIO_Conf==In_PullDown)
	{
		(GPIOStructPtr->GPIO)->ODR&=~(1<<(GPIOStructPtr->GPIO_Pin)); // mise à 0 du bit corresp
		// dans ODR
		GPIOStructPtr->GPIO_Conf=In_PullDown; // même code CNFMOD pour up ou douwn
	}
	
	else if ( GPIOStructPtr->GPIO_Conf==In_PullUp)
	{
		(GPIOStructPtr->GPIO)->ODR|=1<<(GPIOStructPtr->GPIO_Pin); // mise à 1 du bit corresp
		// dans ODR
		GPIOStructPtr->GPIO_Conf=In_PullDown; // même code CNFMOD pour up ou douwn
	}
	
	// Programmation
	if ((GPIOStructPtr->GPIO_Pin <8)) // CRL
	{	
	  (GPIOStructPtr->GPIO)->CRL&=~(0xF<<((GPIOStructPtr->GPIO_Pin)*4));
		(GPIOStructPtr->GPIO)->CRL|=((GPIOStructPtr->GPIO_Conf)<<((GPIOStructPtr->GPIO_Pin)*4));
	}
	else
	{
	  (GPIOStructPtr->GPIO)->CRH&=~(0xF<<((GPIOStructPtr->GPIO_Pin-8)*4));
		(GPIOStructPtr->GPIO)->CRH|=((GPIOStructPtr->GPIO_Conf)<<((GPIOStructPtr->GPIO_Pin-8)*4));
	}
	
}



//***************************************************************************************
//=======================================================================================
//         Fin : Inclusion fcts GPIO perso pour que le module soit autonome
//=======================================================================================
//***************************************************************************************






/*========================================================================================= 
														GESTION ERREURS 
========================================================================================= */

// indicateur erreur transmission
static MyI2C_Err_Enum I2C1_Err,I2C2_Err;

/**
  * @brief  Retourne les erreurs I2C
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @retval Type d'erreur rencontrée , voir MyI2C_Err_Enum
  */

MyI2C_Err_Enum MyI2C_Get_Error(I2C_TypeDef * I2Cx)
{
	if (I2Cx==I2C1) return I2C1_Err;
	else return I2C2_Err;
}






/*========================================================================================= 
														INITIALISATION I2C
========================================================================================= */

/**
  * @brief  Initialise l'interface I2C (1 ou 2) 
  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param char IT_Prio_I2CErr 0 à 15 (utilisé en cas d'erreur, IT courte et non bloquante
  * @retval None
  */
void MyI2C_Init(I2C_TypeDef * I2Cx, char IT_Prio_I2CErr)
{
		MyGPIO_Struct_TypeDef VarGPIOStruct;
    VarGPIOStruct.GPIO=GPIOB;
	
	  #ifdef FixBugBusyFlag
	 
	  // Busy Flag glitch fix from STMF100x ErrataSheet p22 sec 2.11.17 (By M.B.)
	  VarGPIOStruct.GPIO=GPIOB;
		if (I2Cx==I2C1)
		{
			VarGPIOStruct.GPIO_Pin=6; VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SCL ODR=1
		  while(! (VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); // check bit set
			VarGPIOStruct.GPIO_Pin=7; VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
			VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SDA ODR=1
		  while(! (VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit set
		  VarGPIOStruct.GPIO_Pin=6;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR &= ~(1<<(VarGPIOStruct.GPIO_Pin)); // Set SCL ODR=0
		  while((VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit clear
		  VarGPIOStruct.GPIO_Pin=7;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR &= ~(1<<(VarGPIOStruct.GPIO_Pin)); // Set SDA ODR=0
		  while((VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit clear
		  VarGPIOStruct.GPIO_Pin=6;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SCL ODR=1
		  while(!(VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit set
		  VarGPIOStruct.GPIO_Pin=7;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SDA ODR=1
		  while(!(VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit set
			VarGPIOStruct.GPIO_Pin=6;VarGPIOStruct.GPIO_Conf=AltOut_OD; MyGPIO_Init(&VarGPIOStruct);
			VarGPIOStruct.GPIO_Pin=7; VarGPIOStruct.GPIO_Conf=AltOut_OD; MyGPIO_Init(&VarGPIOStruct);
				
	
			I2Cx->CR1 |= I2C_CR1_SWRST; //RESET I2C set
			I2Cx->CR1 &= ~I2C_CR1_SWRST; //RESET I2C clear ***END of fix***
		}
			
		else // I2C2
		{
			VarGPIOStruct.GPIO_Pin=10; VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SCL ODR=1
		  while(! (VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); // check bit set
			VarGPIOStruct.GPIO_Pin=11; VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
			VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SDA ODR=1
		  while(! (VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit set
		  VarGPIOStruct.GPIO_Pin=10;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR &= ~(1<<(VarGPIOStruct.GPIO_Pin)); // Set SCL ODR=0
		  while((VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit clear
		  VarGPIOStruct.GPIO_Pin=11;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR &= ~(1<<(VarGPIOStruct.GPIO_Pin)); // Set SDA ODR=0
		  while((VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit clear
		  VarGPIOStruct.GPIO_Pin=10;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SCL ODR=1
		  while(!(VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit set
		  VarGPIOStruct.GPIO_Pin=11;VarGPIOStruct.GPIO_Conf=Out_OD; MyGPIO_Init(&VarGPIOStruct);
		  VarGPIOStruct.GPIO->ODR |= (1<<(VarGPIOStruct.GPIO_Pin)); // Set SDA ODR=1
		  while(!(VarGPIOStruct.GPIO->IDR & (1<<(VarGPIOStruct.GPIO_Pin)))); //Check bit set
			VarGPIOStruct.GPIO_Pin=10;VarGPIOStruct.GPIO_Conf=AltOut_OD; MyGPIO_Init(&VarGPIOStruct);
			VarGPIOStruct.GPIO_Pin=11; VarGPIOStruct.GPIO_Conf=AltOut_OD; MyGPIO_Init(&VarGPIOStruct);
				
	
			I2Cx->CR1 |= I2C_CR1_SWRST; //RESET I2C set
			I2Cx->CR1 &= ~I2C_CR1_SWRST; //RESET I2C clear ***END of fix***
		}
			
	
	// End fix from STMF100x ErrataSheet p22 sec 2.11.17 (By M.B.)
	
	#endif
	
	
	// INIT GPIO	
	VarGPIOStruct.GPIO_Conf=AltOut_OD;
	 if (I2Cx==I2C1)
	 {
		 I2C1_Err=OK;
		 // Activations d' horloge  I2C1 
		 RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
		 
		 // SCL
		 VarGPIOStruct.GPIO=GPIOB;
		 VarGPIOStruct.GPIO_Pin=6;
		 MyGPIO_Init(&VarGPIOStruct);
		 // SDA
		 VarGPIOStruct.GPIO_Pin=7;
		 MyGPIO_Init(&VarGPIOStruct);
	 }
	else
		{
		 I2C2_Err=OK;
		 // Activations d' horloge  I2C2
		 RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
			
		 // SCL
		 VarGPIOStruct.GPIO=GPIOB;
		 VarGPIOStruct.GPIO_Pin=10;
		 MyGPIO_Init(&VarGPIOStruct);
		 // SDA
		 VarGPIOStruct.GPIO_Pin=11;
		 MyGPIO_Init(&VarGPIOStruct);
	 }
	
	 
	 
	/*---------------- Configuration de l'I2C ----------------*/
	// Réglage à 100kHz FREQ[5..0] = 8 (8MHz) , par exemple ! (2MHz min 36MHz max)
	//                                          dont Tpclk = 1/8 = 125ns
	// Thigh = Tlow = Tpclk* CCR, donc ici 40 pour avoir 5us et donc 100kHz...
	I2Cx->CR2&=~I2C_CR2_FREQ;
	I2Cx->CR2|=36; // FREQ[5..0] = 36 (36MHz) doit être égal la freq ABP1 exactement.
	I2Cx->CCR=180; // en plus std mode (bit F/S à 0) CCCR*1/FAPB1 = 5us Donc CCR = 36M*5u = 180

	I2Cx->TRISE=37; // rise time max 1us (spec philipps I2C1) voir RM008
									// Trise * 1/FAPB1 = 1us soit Rise = 36*1 = 36 (la dic dit qu'il faut ajouter 1)
	 

	

	/*---------------- Configuration des IT I2C ----------------*/
	// Activation IT sur erreur
	I2Cx->CR2|=I2C_CR2_ITERREN;
	 
 	I2Cx->CR1 |= I2C_CR1_PE; // Enable I2C
	/*---------------- Configuration des IT I2C au NVIC-----------*/

	if   (I2Cx==I2C1)
	{
		NVIC_SetPriority(I2C1_ER_IRQn , IT_Prio_I2CErr);
		NVIC_EnableIRQ(I2C1_ER_IRQn );
	}
	else
	{
		NVIC_SetPriority(I2C2_ER_IRQn , IT_Prio_I2CErr);
		NVIC_EnableIRQ(I2C2_ER_IRQn );
	}



}




/*========================================================================================= 
														EMISSION I2C : PutString
========================================================================================= */

/**
	* @brief  |Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |ACK|   pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|ACK| 
|t7|t6|t5|t4|t3|t2|t1|t0|ACK| ...|t7|t6|t5|t4|t3|t2|t1|t0|ACK|Stop Cond|

  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress = PteurMem, adresse de démarrage écriture à l'interieur du slave I2C
  * @param  DataToSend, adresse de la structure qui contient les informations à transmettre 
             voir définition	MyI2C_RecSendData_Typedef				
  * @retval None
  */

void MyI2C_PutString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToSend)
{
	int buffer;
	char * Ptr_Data;
	int Cpt_Byte;
	
	Ptr_Data=DataToSend->Ptr_Data;
	Cpt_Byte=DataToSend->Nb_Data;
	if (I2Cx==I2C1) I2C1_Err=OK;
	else I2C2_Err=OK;

	//==============START + Slave Adr ==================================================
	// attente libération du bus
	//while(LL_I2C_IsActiveFlag_BUSY(I2Cx));
	while((I2Cx->SR1&I2C_SR2_BUSY)==I2C_SR2_BUSY);
	// Start order
	//LL_I2C_GenerateStartCondition(I2Cx);
	I2Cx->CR1 |= I2C_CR1_ACK ; // Set ACK NOT MENTIONED IN REFERENCE MANUAL
	I2Cx->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)
	//-------------EV5  (le start s'est bien passé)  BUSY, MSL and SB flag ----------------
	//while(!LL_I2C_IsActiveFlag_SB(I2Cx));
	while(!(I2Cx->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
		
  // Slave Adress Emission
	buffer = I2Cx->SR1; // Dummy read 
	I2Cx->DR=DataToSend->SlaveAdress7bits<<1; // insertion Write bit
	//LL_I2C_TransmitData8(I2Cx, DataToSend->SlaveAdress7bits<<1); // insertion bit 0 = 0, @paire
	
	//-------------EV6   BUSY, MSL, ADDR, TXE and TRA flags  ---------------------------------
	//while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
	while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
	buffer = I2Cx->SR1;  // Dummy read to clear ADDR
	buffer = I2Cx->SR2;  // Dummy read to clear ADDR
	while ((I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR =0	// ajout perso

	
  //============== Fin phase Addressage ==================================================

	// émission @pteur registre interne
	while (!(I2Cx->SR1 & I2C_SR1_TXE)) {}; //Wait for TX empty TXE=1	
	I2Cx->DR = PteurAdress;		
	//============== emission de N octets =======================================
			
	while (Cpt_Byte>0)
	{
	  //while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
		while (!(I2Cx->SR1 & I2C_SR1_TXE)) {}; //Wait for TX empty TXE=1	
		I2Cx->DR = *Ptr_Data;	
		//LL_I2C_TransmitData8(I2Cx, *Ptr_Data);
		Cpt_Byte--;
		Ptr_Data++;
	}

  //-------------EV8_2   TRA, BUSY, MSL, TXE and BTF flags  ---------------------------------	
	//while(!LL_I2C_IsActiveFlag_BTF(I2Cx));
  while (!(I2Cx->SR1 & I2C_SR1_BTF)) {};
	
	//============== STOP condition ======================================================
	//LL_I2C_GenerateStopCondition(I2Cx);
	I2Cx->CR1 |= I2C_CR1_STOP;
		
		
}






/*========================================================================================= 
														Réception I2C : GetString 
========================================================================================= */

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


void MyI2C_GetString(I2C_TypeDef * I2Cx, char PteurAdress, MyI2C_RecSendData_Typedef * DataToReceive)
{

	char * Ptr_Data;
	int Cpt_Byte;
	int buffer;
	Ptr_Data=DataToReceive->Ptr_Data;


	
	Cpt_Byte=DataToReceive->Nb_Data;
	if (I2Cx==I2C1) I2C1_Err=OK;
	else I2C2_Err=OK;
	
	//==============START + Slave Adr ==================================================
	// attente libération du bus
	//while(LL_I2C_IsActiveFlag_BUSY(I2Cx));
	while((I2Cx->SR1&I2C_SR2_BUSY)==I2C_SR2_BUSY);
	// Start order
	//LL_I2C_GenerateStartCondition(I2Cx);
	I2Cx->CR1 |= I2C_CR1_ACK ; // Set ACK NOT MENTIONED IN REFERENCE MANUAL
	I2Cx->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)
	//-------------EV5  (le start s'est bien passé)  BUSY, MSL and SB flag ----------------
	//while(!LL_I2C_IsActiveFlag_SB(I2Cx));
	while(!(I2Cx->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
		
  // Slave Adress Emission
	buffer = I2Cx->SR1; // Dummy read 
	I2Cx->DR=(DataToReceive->SlaveAdress7bits)<<1; // insertion Write bit
	//LL_I2C_TransmitData8(I2Cx, DataToSend->SlaveAdress7bits<<1); // insertion bit 0 = 0, @paire
	
	//-------------EV6   BUSY, MSL, ADDR, TXE and TRA flags  ---------------------------------
	//while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
	while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
	buffer = I2Cx->SR1;  // Dummy read to clear ADDR
	buffer = I2Cx->SR2;  // Dummy read to clear ADDR
	while ((I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR =0	// ajout perso

	
  //============== Fin phase Addressage ==================================================
	
		
		
		
	//============== émission adress pointer (attente effective fin transm avant restart) ======

	//while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
	while (!(I2Cx->SR1 & I2C_SR1_TXE)) {}; //Wait for TX empty TXE=1	
	I2Cx->DR = PteurAdress;	

	//-------------EV8_2   TRA, BUSY, MSL, TXE and BTF flags  ---------------------------------	
	//while(!LL_I2C_IsActiveFlag_BTF(I2Cx));
  while (!(I2Cx->SR1 & I2C_SR1_BTF)) {};
		
	
		
	// restart en mode read

	
	
	// Ack après réception data (ce mode change au dernier octet donc il faut le répréciser)
	I2Cx->CR1 |= I2C_CR1_ACK ; // Set ACK NOT MENTIONED IN REFERENCE MANUAL

	//==============START + Slave Adr ==================================================
	//LL_I2C_GenerateStartCondition(I2Cx);
	I2Cx->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)
	//-------------EV5  (le start s'est bien passé)  BUSY, MSL and SB flag ----------------
	//while(!LL_I2C_IsActiveFlag_SB(I2Cx));
	while(!(I2Cx->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
	
  // Slave Adress for reception
	// Slave Adress Emission
	buffer = I2Cx->SR1; // Dummy read 
	I2Cx->DR=((DataToReceive->SlaveAdress7bits<<1)|1); // insertion bit 0 = 1, @impaire, lecture

		
	//-------------EV6   BUSY, MSL, ADDR, TXE and TRA flags  ---------------------------------
	//while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
	while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
	buffer = I2Cx->SR1;  // Dummy read to clear ADDR
	buffer = I2Cx->SR2;  // Dummy read to clear ADDR
	while ((I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR =0	// ajout perso
	
  //============== Fin phase Addressage ==================================================

		
  //============== réception de N-1 premiers octets =======================================

	while (Cpt_Byte>1)
	{
		while (!(I2Cx->SR1 & I2C_SR1_RXNE)) {}; //Wait for RXNE
		*Ptr_Data=I2Cx->DR;
		Cpt_Byte--;
		Ptr_Data++;		
	}
  //============== réception dernier octet (attente effective fin transm avant stop) ======
	// config en Nack
	I2Cx->CR1 &= ~I2C_CR1_ACK ; // Set NACK for the last byte

	while (!(I2Cx->SR1 & I2C_SR1_RXNE)) {}; //Wait for RXNE
	*Ptr_Data=I2Cx->DR;

		
	//============== STOP condition ======================================================
	//LL_I2C_GenerateStopCondition(I2Cx);
	I2Cx->CR1 |= I2C_CR1_STOP;
	
}




//==================================================
//
//           INTERRUPTIONS I2c

//=================================================


void I2C1_ER_IRQHandler (void)
{
	if (I2C1->SR1&=I2C_SR1_TIMEOUT==I2C_SR1_TIMEOUT) I2C1_Err=TimeOut;
  else if (I2C1->SR1&=I2C_SR1_AF==I2C_SR1_AF) I2C1_Err=AckFail;
  else if (I2C1->SR1&=I2C_SR1_BERR==I2C_SR1_BERR) I2C1_Err=BusError;
	else 	I2C1_Err=UnknownError;	
}	


void I2C2_ER_IRQHandler  (void)
{
	if (I2C2->SR1&=I2C_SR1_TIMEOUT==I2C_SR1_TIMEOUT) I2C2_Err=TimeOut;
  else if (I2C2->SR1&=I2C_SR1_AF==I2C_SR1_AF) I2C2_Err=AckFail;
  else if (I2C2->SR1&=I2C_SR1_BERR==I2C_SR1_BERR) I2C2_Err=BusError;
	else 	I2C2_Err=UnknownError;
}


