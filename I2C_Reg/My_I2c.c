#include "My_I2c.h"


// Pin I2C1 PB6 SCL PB7 SDA  All pins AltOpen Drain
// I2C1 Remap PB8 SCL PB9 SDA 
// APB1 36 MHz

void My_I2C_Init(I2C_TypeDef *i2c,uint8_t remap, uint8_t SpeedMode)
{
	
	// GPIO Config 
	
	MyGPIO_Struct_TypeDef MyGPIO;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;  //Enable Port B
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  //Enable Port C
	MyGPIO.GPIO=GPIOB;
	if (remap) {
		
			// Busy Flag glitch fix from STMF100x ErrataSheet p22 sec 2.11.17
		  MyGPIO.GPIO_Pin=8; MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SCL ODR=1
		  while(! (MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); // check bit set
			MyGPIO.GPIO_Pin=9; MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
			MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SDA ODR=1
		  while(! (MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit set
		  MyGPIO.GPIO_Pin=8;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR &= ~(1<<(MyGPIO.GPIO_Pin)); // Set SCL ODR=0
		  while((MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit clear
		  MyGPIO.GPIO_Pin=9;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR &= ~(1<<(MyGPIO.GPIO_Pin)); // Set SDA ODR=0
		  while((MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit clear
		  MyGPIO.GPIO_Pin=8;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SCL ODR=1
		  while(!(MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit set
		  MyGPIO.GPIO_Pin=9;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SDA ODR=1
		  while(!(MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit set
			MyGPIO.GPIO_Pin=8; MyGPIO.GPIO_Conf=AltOut_OpenDrain; MyGPIO_Init(&MyGPIO);
			MyGPIO.GPIO_Pin=9; MyGPIO.GPIO_Conf=AltOut_OpenDrain; MyGPIO_Init(&MyGPIO);
			}	
	else {
			
	   
		  // Busy Flag glitch fix from STMF100x ErrataSheet p22 sec 2.11.17
			MyGPIO.GPIO_Pin=6; MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SCL ODR=1
		  while(! (MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); // check bit set
			MyGPIO.GPIO_Pin=7; MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
			MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SDA ODR=1
		  while(! (MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit set
		  MyGPIO.GPIO_Pin=6;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR &= ~(1<<(MyGPIO.GPIO_Pin)); // Set SCL ODR=0
		  while((MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit clear
		  MyGPIO.GPIO_Pin=7;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR &= ~(1<<(MyGPIO.GPIO_Pin)); // Set SDA ODR=0
		  while((MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit clear
		  MyGPIO.GPIO_Pin=6;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SCL ODR=1
		  while(!(MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit set
		  MyGPIO.GPIO_Pin=7;MyGPIO.GPIO_Conf=Out_OpenDrain; MyGPIO_Init(&MyGPIO);
		  MyGPIO.GPIO->ODR |= (1<<(MyGPIO.GPIO_Pin)); // Set SDA ODR=1
		  while(!(MyGPIO.GPIO->IDR & (1<<(MyGPIO.GPIO_Pin)))); //Check bit set
			MyGPIO.GPIO_Pin=6;MyGPIO.GPIO_Conf=AltOut_OpenDrain; MyGPIO_Init(&MyGPIO);
			MyGPIO.GPIO_Pin=7; MyGPIO.GPIO_Conf=AltOut_OpenDrain; MyGPIO_Init(&MyGPIO);
		
		}
	
		i2c->CR1 |= I2C_CR1_SWRST; //RESET I2C set
		i2c->CR1 &= ~I2C_CR1_SWRST; //RESET I2C clear ***END of fix***
		
	 // Registers config PE must be disabled to configure CCR
		
		i2c->CR2 |= 36; // APB1 clock 36MHz
		i2c->TRISE &= ~(I2C_TRISE_TRISE); //Reset  TRISE value
		if (SpeedMode==STD_SPEED) {i2c->CCR = 180; i2c->TRISE |= 36;} // = 1000ns / TClock I2C (36MHz) Datsheet F103 p 71
		if (SpeedMode==FAST_SPEED) {i2c->CCR =45;  i2c->TRISE |= 11;}  // = 300nS / TClock I2C
	 
		i2c->CR1 |= I2C_CR1_PE; // Enable I2C
		if (remap) AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP; //Set MAPR if remap
		
		
}



void My_I2C_MRead(I2C_TypeDef *i2c,uint8_t slaveAdd, uint16_t *readArray, uint8_t nBytes)
{
	uint16_t buffer, count=0;
	
	// Initiate Read Sequence
	
	i2c->CR1 |= I2C_CR1_ACK ; // Set ACK NOT MENTIONED IN REFERENCE MANUAL
	i2c->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)
	
	while(!(i2c->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
	buffer = i2c->SR1; // Dummy read 
  
		
// 1 byte Read 
		
	if (nBytes==1)
			{
			i2c->DR = (slaveAdd<<1) +1;  //Clear SB : Write Slave Address shifted, LSB 1 to read (and 0 to write)
			while (!(i2c->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
			i2c->CR1 &= ~I2C_CR1_ACK ; // NACK : Acknowledge disable
			buffer = i2c->SR1; // Dummy read to clear ADDR
			buffer = i2c->SR2; // Dummy read to clear ADDR
			i2c->CR1 |= I2C_CR1_STOP ; // STOP condition
			while (!(i2c->SR1 & I2C_SR1_RXNE)) {}; //Wait RXNE
			readArray[0]=i2c->DR; //Read data
			}

// 2 Bytes Read			
	else if (nBytes==2) 
			{
			i2c->CR1 |= I2C_CR1_ACK ; // Set ACK
			i2c->CR1 |= I2C_CR1_POS ; // Set POS	
			i2c->DR = (slaveAdd<<1) +1;  //Clear SB : Write Slave Address shifted, LSB 1 to read (and 0 to write)
			while (!(i2c->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
			buffer = i2c->SR1; // Dummy read to clear ADDR
			buffer = i2c->SR2; // Dummy read to clear ADDR
			i2c->CR1 &= ~I2C_CR1_ACK ; // NACK : Acknowledge disable
			while (!(i2c->SR1 & I2C_SR1_BTF)) {}; //Wait BTF
			i2c->CR1 |= I2C_CR1_STOP ; // STOP condition
			readArray[0]=i2c->DR;
			readArray[1]=i2c->DR;
			}
			
// N>2 Bytes Read
		
	else if (nBytes>2) {	
			
			i2c->DR = (slaveAdd<<1) +1;  //Clear SB : Write Slave Address shifted, LSB 1 to read (and 0 to write)
			while (!(i2c->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
		
					buffer = i2c->SR1;  // Dummy read to clear ADDR
					buffer = i2c->SR2;  // Dummy read to clear ADDR
				
					// 3 or more bytes to read remaining
					for (count =0; count <nBytes-3; count++){
					while (!(i2c->SR1 & I2C_SR1_RXNE)) {}; //Wait for RXNE
					readArray[count]=i2c->DR;							//read DR, clear RXNE
					i2c->CR1 |= I2C_CR1_ACK ;	//ACK
					}
					
					// 3 last Bytes cf RM008p726
					while (!(i2c->SR1 & I2C_SR1_RXNE)) {}; //Wait for RXNE NO DR Read
					i2c->CR1 |= I2C_CR1_ACK ;	//ACK
						
					while (!(i2c->SR1 & I2C_SR1_BTF)) {}; //Wait for BTF, don't read DR (N-2 is in DR and N-1 in Shift Register)
					i2c->CR1 &= ~I2C_CR1_ACK ; // clear ACK bit (NACK generation)
					readArray[count]=i2c->DR; // Read N-2 Byte, data N being received, N-1 goes to DR
					i2c->CR1 |= I2C_CR1_STOP ; // STOP condition
					readArray[count+1]=i2c->DR; //Read N-1
					while (!(i2c->SR1 & I2C_SR1_RXNE)) {}; //Wait for RXNE
					readArray[count+2]=i2c->DR; // Read last byte
						
			}

}

void My_I2C_MWrite(I2C_TypeDef *i2c,uint8_t slaveAdd, uint16_t *writeArray, uint8_t nBytes, uint8_t stop)
{


	uint16_t buffer, count=0;
	
	
	// Initiate Write Sequence
	
	i2c->CR1 |= I2C_CR1_ACK ; // Set ACK NOT MENTIONED IN REFERENCE MANUAL
	i2c->CR1 |= I2C_CR1_START; //Start condition (see RM008 p 759)

	while(!(i2c->SR1 & I2C_SR1_SB )){};	//Wait for SB flag	
	buffer = i2c->SR1; // Dummy read 
	i2c->CR1 &= ~I2C_CR1_ACK ; // NACK : Acknowledge disable
	i2c->DR = (slaveAdd<<1) +0;  //Clear SB : Write Slave Address shifted, LSB 0 = write
			while (!(i2c->SR1 & I2C_SR1_ADDR)) {};//Wait for ADDR
		
			buffer = i2c->SR1;  // Dummy read to clear ADDR
			buffer = i2c->SR2;  // Dummy read to clear ADDR
				
			//Send N-1 bytes
			for (count =0; count <nBytes-1; count++){
			i2c->DR = writeArray[count];	
			while (!(i2c->SR1 & I2C_SR1_TXE)) {}; //Wait for TX empty TXE=1				
			}
			
			// Last Byte
      		
			i2c->DR = writeArray[count];
			while (!(i2c->SR1 & I2C_SR1_TXE)) {};
			if(stop) i2c->CR1 |= I2C_CR1_STOP ; // STOP condition if STOP is <> 0
 

}

void My_I2C_IsBusy(I2C_TypeDef *i2c)
{
 while((i2c->SR2 & I2C_SR2_BUSY));
}


