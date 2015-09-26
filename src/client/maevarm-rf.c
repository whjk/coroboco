#include "maevarm-rf.h"

// default packet_size of 8 bytes
static char packet_size = 0x08;

// set up the spi port in master mode with the polarity options (etc.) that the 24L01 requires. Also set DDR of the CE pin of rf module.
void SPIsetup()
{
	//Make sure Power Save didn't turn SPI off
 	PRR0 &= ~(1 << PRSPI);

	// Set MOSI and SCK and SS output, all others input. SS MUST be config. to output 
	// Also configure the CE pin of the rf module as an output
	DDR_SPI |= (1 << DD_MOSI) | (1<<DD_SCK) | (1<<DD_SS);

	// Enable SPI, Master, set clock rate sys_clk/16
//	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);

	// Enable SPI, Master, set clock rate sys_clk/128
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);

	//PAUSE to let the wireless chip go through initialization 
	_delay_ms(200);
}

// function to write a data register on the 24L01 RF chip. 
// writeReg - the register we want to write data to
// writeDat - the storage container for the data we plan on writing
// numBytes - the number of bytes to write. !! The writeDat storage container must be at least this size in bytes (char's)
void RFwriteReg(char writeReg, char* writeDat, int numBytes)
{

	// turn the ss channel low to begin transmission
	PORT_RF &= ~(1<<PRF_CS);

	// send 1 byte out the spi port to request write of writeReg
	// in order to do this we add 0x20 to our write register as per the format of the 23L01 datasheet
	SPDR = W_REGISTER + writeReg;

	// wait for transmission to finish
	while(!(SPSR & (1<<SPIF) ) );

	// write the number of bytes we want from writeDat
	int i;
	for(i = 0; i < numBytes; i++)
	{
		// send 1 byte out the spi port. this is the data we want to write to the RF chip
		SPDR = writeDat[i];

		// wait for transmission to finish
		while(!(SPSR & (1<<SPIF) ) );
	}

	// turn the ss channel high to end transmission
	PORT_RF |= (1<<PRF_CS);
}





// function to fill the transfer (TX) buffer with data we wish to transmit wirelessly
// txDat - an appropriately sized storage container containing the data we plan on pushing into the TX register
// numBytes - the number of bytes to push in back. !! The txDat storage container must be at least this size in bytes (char's)
void RFfillTransferBuffer( char* txDat, int numBytes )
{
	// turn the ss channel low to begin transmission
	PORT_RF &= ~(1<<PRF_CS);

	// send 1 byte out the spi port that indicates we want to fill the TX buffer
	SPDR = W_TX_PAYLOAD;

	// wait for transmission to finish
	while(!(SPSR & (1<<SPIF) ) );

	// write the number of bytes we want to write out to the TX buffer
	int i;
	for(i = 0; i < numBytes; i++)
	{
		// send 1 byte out the spi port. this is the data we want to write to the RF chip
		SPDR = txDat[i];

		// wait for transmission to finish
		while(!(SPSR & (1<<SPIF) ) );
	}

	// turn the ss channel high to end transmission
	PORT_RF |= (1<<PRF_CS);
}


// function to read a data register from the 24L01 RF chip. 
// readReg - the register we want to read data from
// retDat - an appropriately sized storage container for the data we plan on getting back
// numBytes - the number of bytes to read back. !! The retDat storage container must be at least this size in bytes (char's)
void RFreadReg(char readReg, char* retDat, int numBytes)
{

	// turn the ss channel low to begin transmission
	PORT_RF &= ~(1<<PRF_CS);

	// send 1 byte out the spi port to request write of writeReg
	SPDR = R_REGISTER + readReg;

	// wait for transmission to finish
	while(!(SPSR & (1<<SPIF) ) );

	// read the number of bytes we intend to receive into retDat
	int i;
	for(i = 0; i < numBytes; i++)
	{
		// send 1 byte out the spi port. this is a dummy send just to read the incoming MISO data from our read request above
		SPDR = 0xFF;

		// wait for transmission to finish
		while(!(SPSR & (1<<SPIF) ) );

		// read off the contents of our return data that is in SPDR (the value we requested to read). This came from MISO
		retDat[i] = SPDR;
	}

	// turn the ss channel high to end transmission
	PORT_RF |= (1<<PRF_CS);
}


// put the chip into receiving mode
void RFsetRxAddr(char * recvAddr, int numBytes)
{
	RFwriteReg(RX_ADDR_P1,recvAddr,numBytes);
}


// flush the tx buffer
void RFflushTXBuffer()
{

	// turn the ss channel low to begin transmission
	PORT_RF &= ~(1<<PRF_CS);

	// send 1 byte out the spi port to request write of writeReg
	SPDR = FLUSH_TX;

	// wait for transmission to finish
	while(!(SPSR & (1<<SPIF) ) );

	// turn the ss channel high to end transmission
	PORT_RF |= (1<<PRF_CS);
}

// flush the rx buffer
void RFflushRXBuffer()
{

	// turn the ss channel low to begin transmission
	PORT_RF &= ~(1<<PRF_CS);

	// send 1 byte out the spi port to request write of writeReg
	SPDR = FLUSH_RX;

	// wait for transmission to finish
	while(!(SPSR & (1<<SPIF) ) );

	// turn the ss channel high to end transmission
	PORT_RF |= (1<<PRF_CS);
}

// put the chip into receiving mode
void RFstartReceiving()
{
	char writeDat[1] = { CONFIG_VAL | (1<<PWR_UP) | (1<<PRIM_RX) };

	// turn the PWR_UP and PRIM_RX bits high
	RFwriteReg( CONFIG, writeDat, 1);

	// wait a millisecond 
	_delay_ms(2);

	// turn pin CE high
	PORT_RF |= (1<<PRF_CE);
}

// take the chip out of receiving mode
void RFstopReceiving()
{
	// turn pin CE low
	PORT_RF &= ~(1<<PRF_CE);
}

// transmit txDat wirelessly and repeat until we receive verification that the packet was received. Returns chip to receive state when done
char RFtransmitUntil( char* txData, char* destAddr, char txTries)
{
	char txAttempt = 0;
	char success = 0;
	char tempWrite[1] = { 0x00 };
	
	// take out of receiving mode
	RFstopReceiving();

	
	// repeat transmission until the TX bit goes high (verifies receipt) or we've tried txTries times
	while( (!success) &&  (txAttempt < txTries)  )
	{

		RFflushTXBuffer();
		
		// clear the TX transmission bit
		tempWrite[0] = (1<<TX_DS);
		RFwriteReg(STATUS, tempWrite, 1);


		// increment the attempt counter
		txAttempt++; 		
		
		// set up the destination transmit address
		RFwriteReg(TX_ADDR,destAddr,5);

		// set up the destination recive address for auto-acknowledgement
		RFwriteReg(RX_ADDR_P0,destAddr,5);

		// setup our write data to configure the chip into TX mode
		char writeDat[1] = { CONFIG_VAL | (1<<PWR_UP) };

		// turn the PWR_UP high and PRIM_RX bit low for transmitting
		RFwriteReg( CONFIG, writeDat, 1);

		//write data to TX register for outputting
		RFfillTransferBuffer(txData,packet_size);

		// turn pin CE high
		PORT_RF |= (1<<PRF_CE);

		// wait 2 millisecond to make sure transfer fires
		_delay_ms(2);

		// end transmission by pulling CE low
		PORT_RF &= ~(1<<PRF_CE);
		
		// delay so we have time to receive an ACK response
		_delay_ms(4);
		
		// temp variable to store our STATUS register state
		char tempStatus[1] = {0x00};
		RFreadReg(STATUS,tempStatus,1);

		// check for acknowledgement from the receiving node
		//success = tempStatus[0];
		success = ( tempStatus[0] & (1<<TX_DS));

		// clear any MAX_RT bits transmission bit
		tempWrite[0] = (1<<MAX_RT);
		RFwriteReg(STATUS, tempWrite, 1);	
	}
		
	// if the TX bit is high clear the TX transmission bit
	if(success)
	{
		tempWrite[0] =  (1<<TX_DS);
		RFwriteReg(STATUS, tempWrite, 1);
	}

	// turn receiving mode back on 
	RFstartReceiving();

	if(success)
	{
		return txAttempt;
	}
	else
	{
		return 0;
	}
}


// transmit txDat wirelessly and leave the chip in receive mode when done. Function only fires 1 packet and does not check if data was received.
void RFtransmit( char* txData, char* destAddr)
{
	RFtransmitUntil(txData,destAddr,1);
}


char RFRXdataReady()
{
	// dummy write data
	char retDat[1] = {0xFF};

	RFreadReg(STATUS,retDat,1);

	if( retDat[0] & (1<<RX_DR) )
	{
		return 1;
	}

	else
	{
		return 0;
	}

}


int RFRXbufferEmpty()
{
	// dummy write data
	char retDat[1] = {0xFF};

	RFreadReg(STATUS,retDat,1);

	if( retDat[0] & 0x0E )
	{
		return 1;
	}

	else
	{
		return 0;
	}
}

// read data out of the receive FIFO and turn off the data-ready flag RX_DR if the buffer is empty
// this function stores the previous value of the CONFIG register, which indicated whether we were transmitting or receiving prior to this function call, and restores this state after reading
// NOTE: it is not possible to receive or transmit new packets while readings, all packets will be lost during this time
void RFreadRXFIFO( char* retDat)
{
	// take out of receiving mode
	RFstopReceiving();

	// dummy write data
	retDat[0] = 0xFF;

	// turn the ss channel low to begin transmission
	PORT_RF &= ~(1<<PRF_CS);

	// send 1 byte out the spi port that indicates we want to read the RX FIFO
	SPDR = R_RX_PAYLOAD;

	// wait for transmission to finish
	while(!(SPSR & (1<<SPIF) ) );

	// read the number of bytes we want to read out to the RX buffer
	int i;
	for(i = 0; i < packet_size; i++)
	{
		// read 1 byte out the spi port
		SPDR = retDat[i];

		// wait for transmission to finish
		while(!(SPSR & (1<<SPIF) ) );

		// store the returned data
		retDat[i] = SPDR;
	}


	// turn the ss channel high to end transmission
	PORT_RF |= (1<<PRF_CS);

	_delay_ms(2);

	// if there is no new data ready to be read then turn the RX_DR data ready pin low
	if(RFRXbufferEmpty())
	{
		char writeDat[1] = { (1<<RX_DR) };
		RFwriteReg(STATUS,writeDat,1);
	}

	// put back in receiving mode
	RFstartReceiving();
}


int RFRXbufferFull()
{
	// dummy write data
	char retDat[1] = {0xFF};

	RFreadReg(FIFO_STATUS,retDat,1);

	if(retDat[0] & (1<<RX_FULL))
	{
		return 1;
	}

	else
	{
		return 0;
	}
}


// clear out the FIFO and return the last received packet
void RFreceive(char * buffer)
{
	while(RFRXdataReady())
	{
		RFreadRXFIFO(buffer);
	}
}


// function to setup the rf chip registers for communication and boot it up in receiving mode
void RFsetup(char * recvAddr, char packet_length)
{
	// override the default packet_size
	packet_size = packet_length;
	
	_delay_ms(100);
	
	//setup the SPI port for use with the 24L01 chip
	SPIsetup();

	//setup the RF CE and CS pins as outputs
	DDR_RF |= (1<<DDRRF_CE) | (1<<DDRRF_CS);

	//set the config register up
	char writeDat[1] = {CONFIG_VAL};
	RFwriteReg(CONFIG,writeDat,1);

	//enable PACKET_SIZE byte collection on pipe 1
	writeDat[0]  = packet_size;
	RFwriteReg(RX_PW_P1,writeDat,1);

	//turn on auto acknowledgement on first two pipes
	writeDat[0] = 0x03;
	RFwriteReg(EN_AA,writeDat,1);

	//turn off auto retransmit
	writeDat[0] = 0x00;
	RFwriteReg(SETUP_RETR,writeDat,1);		

	//enable receive on pipe 0 and 1  (this happens by default)
	//writeDat[0] = (1<<ERX_P0) | (1<<ERX_P1);
	//RFwriteReg(EN_RXADDR,writeDat,1);	
	
	//setup address size as 5 bytes (probably deafult setup is already OK)
	//writeDat[0] = 0x03;
	//RFwriteReg(SETUP_AW,writeDat,1);	

	// write receive address to pipe1
	RFsetRxAddr(recvAddr, 5);

	// turn the receiver on
	RFstartReceiving();
}