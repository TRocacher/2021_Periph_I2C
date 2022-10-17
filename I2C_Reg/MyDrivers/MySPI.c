/*
 * MySPI.h
 *
 *  Created on: 19 Juin. 2021
 *      Author: Thierry Rocacher
 *
 *  ======= CONTEXTE DU LOGICIEL ===================
 *  CIBLE : stm32F103 (Nucléo board)
 *  IDE : KEIL V5.30.0 via fichier .ioc généré par CubeMx 6.2.1 (en standalone) 
 *  CONTEXTE : voilier, pilotage Accéléro ADXL345 par SPI
 *  DEPENDENCES : Le code s'appuie sur les registres du stm32F103RB donc
 *  essentiellement sur stm32F103xB.h via stm32F10xx.h (voir \Drivers\CMSIS\Device\ST\STM32F1xx\Include)
 *  LOCALISATION DES .h :
 *				- stm32F10xx.h et stm32f103xb.h : \Drivers\CMSIS\Device\ST\STM32F1xx\Include
 *				
 *  Le codage s'appuie sur les registres directement.
 *
 */


#include "MySPI.h"

/******************************************************************************
*  PRIVATE Variables & fct
*
******************************************************************************/

SPI_TypeDef * ActiveSPI;
void MySPI_Set_NSS(void);
void MySPI_Clear_NSS(void);


/******************************************************************************
*  PUBLIC FUNCTION
*  See RM008.pdf RM
******************************************************************************/


/**
  * @brief 
  */


void MySPI_Init(SPI_TypeDef * Spi)
{
	// activation horloge
	if (Spi==SPI1)	RCC->APB2ENR|= RCC_APB2ENR_SPI1EN;
	else RCC->APB1ENR|=RCC_APB1ENR_SPI2EN;
	
	/*
	Réglage IO:
	- SCLK Alternate Out ppull (CNF_MOD = 0xB)
	- MOSI Alt Out ppull
	- MISO input  (CNF_MOD = 0x4)
	- CSS Output Ppull  (CNF_MOD = 0x3)

	**IO SPI 1**
	SPI1_NSS PA4
	SPI1_SCK PA5
	SPI1_MISO  PA6  
	SPI1_MOSI  PA7  

	**IO SPI 2**
	SPI2_NSS PB12
	SPI2_SCK PB13
	SPI2_MISO  PB14  
	SPI2_MOSI  PB15  
	*/
	
	ActiveSPI=Spi;
	if (ActiveSPI==SPI1)
	{
		RCC->APB2ENR|= RCC_APB2ENR_IOPAEN;
		//SPI1_NSS PA4 Outpp
		GPIOA->CRL&=~(0xF<<(4*4));
		GPIOA->CRL|=0x3<<(4*4);
		//SPI1_SCK PA5 Alt Out ppull
		GPIOA->CRL&=~(0xF<<(5*4));
		GPIOA->CRL|=0xB<<(5*4);
		//SPI1_MISO  PA6  input floating
		GPIOA->CRL&=~(0xF<<(6*4));
		GPIOA->CRL|=0x4<<(6*4);
		//SPI1_MOSI  PA7 Alt Out ppull
		GPIOA->CRL&=~(0xFUL<<(7*4));
		GPIOA->CRL|=0xBUL<<(7*4);
	}
	else 
	{
		RCC->APB2ENR|= RCC_APB2ENR_IOPBEN;
		//SPI2_NSS PB12 Outpp
		GPIOB->CRH&=~(0xF<<((12%8)*4));
		GPIOB->CRH|=0x3<<((12%8)*4);
		//SPI2_SCK PB13 Alt Out ppull
		GPIOB->CRH&=~(0xF<<((13%8)*4));
		GPIOB->CRH|=0xB<<((13%8)*4);
		//SPI2_MISO  PB14  input floating
		GPIOB->CRH&=~(0xF<<((14%8)*4));
		GPIOB->CRH|=0x4<<((14%8)*4);
		//SPI2_MOSI  PB15 Alt Out ppull
		GPIOB->CRH&=~(0xFUL<<((15%8)*4));
		GPIOB->CRH|=0xBUL<<((15%8)*4);
	}
	// Config deux fils
	ActiveSPI->CR1&=~SPI_CR1_BIDIMODE;
	// Baud rate à 72MHz/256 = 281.25kHz (max 400hz sample pour IMU ADXL345)
	// SPI1 sur APB2 rapide 72MHZ
	// SPI2 sur APB1, lent, 36MHz
	if (ActiveSPI==SPI1)
	{
		ActiveSPI->CR1&=~(0x7<<3);// Chp BR
		ActiveSPI->CR1|=0x7<<3;
	}
	else
	{
		ActiveSPI->CR1&=~(0x7<<3);// Chp BR
		ActiveSPI->CR1|=0x6<<3;
	}
	//CPOL=1, CPHA=1 (repos = '1', sample on rising edge)
	ActiveSPI->CR1|=SPI_CR1_CPHA;
	ActiveSPI->CR1|=SPI_CR1_CPOL;
	// 8 bits mode
	ActiveSPI->CR1&=~SPI_CR1_DFF;
	// LSB first
	ActiveSPI->CR1&=~SPI_CR1_LSBFIRST;
	
	// Gestion SSM soft (suivre la doc SSM=SSI=1)
	ActiveSPI->CR1|=SPI_CR1_SSM;
	ActiveSPI->CR1|=SPI_CR1_SSI;
	
	// Master mode
	ActiveSPI->CR1|=SPI_CR1_MSTR;
	// Enable SPI module
	ActiveSPI->CR1|=SPI_CR1_SPE;	
	
	MySPI_Set_NSS();
	while ((ActiveSPI->SR&SPI_SR_BSY)==SPI_SR_BSY);
}


void MySPI_Send(char ByteToSend)
{
	char poubelle;
	if ((ActiveSPI==SPI1)||(ActiveSPI==SPI2))
	{
		// attendre que le buffer soit vide
		while ((ActiveSPI->SR&SPI_SR_TXE)!=SPI_SR_TXE);
		ActiveSPI->DR=ByteToSend;
		//while ((ActiveSPI->SR&SPI_SR_BSY)==SPI_SR_BSY);
		while ((ActiveSPI->SR&SPI_SR_RXNE)!=SPI_SR_RXNE);
		
	}
	poubelle=ActiveSPI->DR; // lecture du byte reçu qui est bidon.
	// nécessaire pour vider le buffer lors d'une lecture.
}

char MySPI_Read(void) 
{
	char ReceivedByte;
	
	if ((ActiveSPI==SPI1)||(ActiveSPI==SPI2))
	{
		// attendre que le buffer soit vide
		while ((ActiveSPI->SR&SPI_SR_TXE)!=SPI_SR_TXE);
		ActiveSPI->DR=0;
		while ((ActiveSPI->SR&SPI_SR_RXNE)!=SPI_SR_RXNE);
		// le buffer est forcément plein
		ReceivedByte=ActiveSPI->DR;
	}
	
	return ReceivedByte;
}



void MySPI_Set_NSS(void)
{
	if (ActiveSPI==SPI1) GPIOA->BSRR=GPIO_BSRR_BS4;
	else GPIOB->BSRR=GPIO_BSRR_BS12;
}

void MySPI_Clear_NSS(void)
{
	if (ActiveSPI==SPI1) GPIOA->BSRR=GPIO_BSRR_BR4;
	else GPIOB->BSRR=GPIO_BSRR_BR12;
}
