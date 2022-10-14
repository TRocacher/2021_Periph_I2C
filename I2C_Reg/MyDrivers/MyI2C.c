#include "MyI2C.h"


/* ======================================================================================

//by periph team

Emission et réception en mode maître bloquant -> à lancer en background pour pouvoir
être interrompus par une autre interruption.

==========================================================================================*/





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

#define I2C_CNFMOD_ClrField 0xF
#define I2C_ALT_OD_CNFMOD 0xF
#define I2C_OD_CNFMOD 0x7

int SCL;
int SDA;
__IO uint32_t * CRLH;

static void (*PtrFct) (void);
	
void MyI2C_Init(I2C_TypeDef * I2Cx, char IT_Prio_I2CErr, void (*ITErr_function) (void))
{
		PtrFct=ITErr_function;
		RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
	  // Busy Flag glitch fix from STMF100x ErrataSheet p22 sec 2.11.17 (By M.B.)
		if (I2Cx==I2C1)
		{
			I2C1_Err=OK;
		 // Activations d' horloge  I2C1 
			RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
			SCL=6;SDA=7;
			CRLH=&(GPIOB->CRL);
		}
		else
		{
			I2C2_Err=OK;
			// Activations d' horloge  I2C2
			RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
			SCL=10;SDA=11;
			CRLH=&(GPIOB->CRH);
		}
		//2 Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
    *CRLH&=~(I2C_CNFMOD_ClrField<<((SCL%8)*4)|I2C_CNFMOD_ClrField<<((SDA%8)*4));		
		*CRLH |= (I2C_OD_CNFMOD<<((SCL%8)*4)|I2C_OD_CNFMOD<<((SDA%8)*4));
		GPIOB->ODR |=(1<<SCL|1<<SDA);
		//3 Check SCL and SDA High level in GPIOx_IDR
		while(! (GPIOB->IDR & (1<<SCL))); // check bit set
		while(! (GPIOB->IDR & (1<<SDA))); // check bit set
		//4 Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
		*CRLH&=~(I2C_CNFMOD_ClrField<<((SDA%8)*4));		
		*CRLH |= (I2C_OD_CNFMOD<<((SDA%8)*4));
		GPIOB->ODR &=~(1<<SDA);	
		//5 Check SDA Low level in GPIOx_IDR.
		while((GPIOB->IDR & (1<<SDA))); // check bit clear
		//6 Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
		*CRLH&=~(I2C_CNFMOD_ClrField<<((SCL%8)*4));		
		*CRLH |= (I2C_OD_CNFMOD<<((SCL%8)*4));
		GPIOB->ODR &=~(1<<SCL);
    //7 Check SCL Low level in GPIOx_IDR.
		while((GPIOB->IDR & (1<<SCL)));		// check bit clear	
		//8 Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to  GPIOx_ODR).
		*CRLH&=~(I2C_CNFMOD_ClrField<<((SCL%8)*4));		
		*CRLH |= (I2C_OD_CNFMOD<<((SCL%8)*4));
		GPIOB->ODR |=(1<<SCL);
		//9 Check SCL High level in GPIOx_IDR
		while(! (GPIOB->IDR & (1<<SCL))); // check bit set
		// 10 Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR)
		*CRLH&=~(I2C_CNFMOD_ClrField<<((SDA%8)*4));		
		*CRLH |= (I2C_OD_CNFMOD<<((SDA%8)*4));
		GPIOB->ODR |=(1<<SDA);
		//11 Check SDA High level in GPIOx_IDR.
		while(! (GPIOB->IDR & (1<<SDA))); // check bit set		
		//12 Configure the SCL and SDA I/Os as Alternate function Open-Drain.
		*CRLH&=~(I2C_CNFMOD_ClrField<<((SCL%8)*4)|I2C_CNFMOD_ClrField<<((SDA%8)*4));		
		*CRLH |= (I2C_ALT_OD_CNFMOD<<((SCL%8)*4)|I2C_ALT_OD_CNFMOD<<((SDA%8)*4));		
	  // 13. Set SWRST bit in I2Cx_CR1 register.
		I2Cx->CR1 |= I2C_CR1_SWRST; //RESET I2C set
		//14. Clear SWRST bit in I2Cx_CR1 register
		I2Cx->CR1 &= ~I2C_CR1_SWRST; //RESET I2C clear ***END of fix***
		//15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register	
	
	  // End fix from STMF100x ErrataSheet p22 sec 2.11.17 (By M.B.)	
		

	 
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
	* @brief|Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |Slave ACK|
					|pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|Slave ACK| 
					|d7|d6|d5|d4|d3|d2|d1|d0|Slave ACK| (data 1)
					.....
					|d7|d6|d5|d4|d3|d2|d1|d0|Salve ACK|Stop Cond| (data N) 

  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress = adresse de démarrage écriture à l'interieur du slave I2C
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
	// attente libération du bus (Busy Flag)
	while((I2Cx->SR1&I2C_SR2_BUSY)==I2C_SR2_BUSY);
	// Start order
	I2Cx->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)
	//-------------EV5  (le start s'est bien passé)  BUSY, MSL and SB flag -------------
	while(!(I2Cx->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
	buffer = I2Cx->SR1; // Dummy read 
	
  // Slave Adress Emission		
	I2Cx->DR=DataToSend->SlaveAdress7bits<<1; // insertion Write bit
		
	//-------------EV6   BUSY, MSL, ADDR, TXE and TRA flags  ---------------------------
	while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
	buffer = I2Cx->SR1;  // Dummy read to clear ADDR
	buffer = I2Cx->SR2;  // Dummy read to clear ADDR
	//============== Fin phase Addressage ==============================================
		
  //============== Emission @ pteur interne ==========================================
	// émission @pteur registre interne
	while (!(I2Cx->SR1 & I2C_SR1_TXE)) {}; //Wait for TX empty TXE=1	
	I2Cx->DR = PteurAdress;		
		
	//============== emission de N octets ==============================================
	while (Cpt_Byte>0)
	{
	  //while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
		while (!(I2Cx->SR1 & I2C_SR1_TXE)) {}; //Wait for TX empty TXE=1	
		I2Cx->DR = *Ptr_Data;	
		//LL_I2C_TransmitData8(I2Cx, *Ptr_Data);
		Cpt_Byte--;
		Ptr_Data++;
	}

  //-------------EV8_2   TRA, BUSY, MSL, TXE and BTF flags  ----------------------------
  while (!(I2Cx->SR1 & I2C_SR1_BTF)) {};
	
	//============== STOP condition ======================================================
	I2Cx->CR1 |= I2C_CR1_STOP;	
}





/*========================================================================================= 
														Réception I2C : GetString 
========================================================================================= */

/**
	* @brief  |Start Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =0 |Slave ACK|
						|pt7|pt6|pt5|pt4|pt3|pt2|pt1|pt0|Slave ACK| 
						|ReStart Cond   |@6|@5|@4|@3|@2|@1|@0| Wr =1 |Slave ACK|
						|d7|d6|d5|d4|d3|d2|d1|d0|Master ACK| (data 1)
						.....
						|d7|d6|d5|d4|d3|d2|d1|d0|Master NACK|Stop Cond| (data N)

  * @param  I2Cx: where x can be 1 or 2 to select the I2C peripheral.
  * @param  PteurAdress = adresse de démarrage lecture à l'interieur du slave I2C
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
	while((I2Cx->SR1&I2C_SR2_BUSY)==I2C_SR2_BUSY);
	// Start order
	I2Cx->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)
	//-------------EV5  (le start s'est bien passé)  BUSY, MSL and SB flag -------------
	while(!(I2Cx->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
	buffer = I2Cx->SR1; // Dummy read 
		
  // Slave Adress Emission
	I2Cx->DR=(DataToReceive->SlaveAdress7bits)<<1; // insertion Write bit
	
	//-------------EV6   BUSY, MSL, ADDR, TXE and TRA flags  ---------------------------
	while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
	buffer = I2Cx->SR1;  // Dummy read to clear ADDR
	buffer = I2Cx->SR2;  // Dummy read to clear ADDR
  //============== Fin phase Addressage ==============================================
	
		
		
		
	//======== émission adress pointer (attente effective fin transm avant restart) ====

	while (!(I2Cx->SR1 & I2C_SR1_TXE)) {}; //Wait for TX empty TXE=1	
	I2Cx->DR = PteurAdress;	

	//-------------EV8_2   TRA, BUSY, MSL, TXE and BTF flags  ---------------------------	
  while (!(I2Cx->SR1 & I2C_SR1_BTF)) {};
	
		
	// restart en mode read
	// Ack après réception data (ce mode change au dernier octet donc il faut le répréciser)
	I2Cx->CR1 |= I2C_CR1_ACK ; // Set ACK NOT MENTIONED IN REFERENCE MANUAL

	//==============START + Slave Adr ==================================================
	I2Cx->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)
	//-------------EV5  (le start s'est bien passé)  BUSY, MSL and SB flag -------------
	while(!(I2Cx->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
	buffer = I2Cx->SR1; // Dummy read
		
  // Slave Adress for reception
	I2Cx->DR=((DataToReceive->SlaveAdress7bits<<1)|1); // insertion bit 0 = 1, @impaire, lecture
		
	//-------------EV6   BUSY, MSL, ADDR, TXE and TRA flags  ----------------------------
	while (!(I2Cx->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
	buffer = I2Cx->SR1;  // Dummy read to clear ADDR
	buffer = I2Cx->SR2;  // Dummy read to clear ADDR
  //============== Fin phase Addressage ===============================================

		
  //============== réception de N-1 premiers octets ===================================

	while (Cpt_Byte>1)
	{
		while (!(I2Cx->SR1 & I2C_SR1_RXNE)) {}; //Wait for RXNE
		*Ptr_Data=I2Cx->DR;
		Cpt_Byte--;
		Ptr_Data++;		
	}
  //============== réception dernier octet (attente effective fin transm avant stop) ==
	// config en Nack : le Master ne doit pas acquitter le dernier octet
	I2Cx->CR1 &= ~I2C_CR1_ACK ; // Set NACK for the last byte

	while (!(I2Cx->SR1 & I2C_SR1_RXNE)) {}; //Wait for RXNE
	*Ptr_Data=I2Cx->DR;

		
	//============== STOP condition ======================================================
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
	PtrFct();
}	


void I2C2_ER_IRQHandler  (void)
{
	if (I2C2->SR1&=I2C_SR1_TIMEOUT==I2C_SR1_TIMEOUT) I2C2_Err=TimeOut;
  else if (I2C2->SR1&=I2C_SR1_AF==I2C_SR1_AF) I2C2_Err=AckFail;
  else if (I2C2->SR1&=I2C_SR1_BERR==I2C_SR1_BERR) I2C2_Err=BusError;
	else 	I2C2_Err=UnknownError;
	PtrFct();
}


