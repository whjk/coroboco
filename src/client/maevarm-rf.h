// -----------------------------------------------------------------------------
// MAEVARM wireless module
// version: 1.4.0
// date: October 15, 2010
// authors: J. Romano and J. Fiene
// -----------------------------------------------------------------------------

#ifndef _RF24L01_HELPER
#define _RF24L01_HELPER

// Provides the following public functions:
 
void RFsetup(char * recvAddr, char packet_length);
// initialize the wireless node and place into receive mode
// - recvAddr is a pointer to a 5-element char array corresponding to your device's address
// - packet_length is a value between 0 and 32

char RFRXdataReady();
// check to see if a message has been received
// - returns 1 if data, 0 if empty

void RFreceive(char * buffer);
// read the latest data from receive buffer
// - retDat - pointer to an n-element char array storage container 

void RFtransmit( char* txData, char* destAddr);
// suspend receive mode and transmit a char array to a specific wireless node
// will only send one packet, and does not check for receipt
// returns to receive mode when finished
// - txData is a pointer to an n-element char array
// - destAddr is a pointer to a 5-element char array corresponding to the receiver's device address

char RFtransmitUntil( char* txData, char* destAddr, char txTries);
// suspend receive mode and transmit a char array to a specific wireless node
// if no acknowledgement is returned from the other node, it will retransmit up to txTries times
// returns 0 if unsuccessful, or the number of tries it took before success
// returns to receive mode when finished
// - txData is a pointer to an n-element char array
// - destAddr is a pointer to a 5-element char array corresponding to the receiver's device address
// - txTries is the maximum number of retransmit attempts

// -----------------------------------------------------------------------------

//basic includes
#include <avr/io.h>
#include <util/delay.h>

// Pins that the 24L01 connects to the MEAMAVR board on
#define PORT_RF    PORTB		// the port used by the RF chip (MOSI,MISO,CLK)
#define DDR_RF     DDRB			// the data register used by the RF chip
#define DDRRF_CE   DDB4			// the data register used by the CE pin of the RF chip
#define DDRRF_CS   DDB0			// the data register used as the CS pin of the RF chip

// Using the RED SPARKFUN modules, we use these pins (swap if using the little green boards)
#define PRF_CS	   PORTB4	  	// the port pin used as the CS pin of the RF chip
#define PRF_CE	   PORTB0		// the port pin used as the CE pin of the RF chip

// Pins that are used for SPI on the MEAMAVR board
#define PORT_SPI    PORTB		// the port that corresponds to the SPI pins of our 32UF AVR chip
#define DDR_SPI     DDRB		// the data register that corresponds to the SPI of our 32UF AVR chip
#define DD_MISO     DDB3		// the data register that corresponds with the MISO SPI pin of the 32UF AVR chip
#define DD_MOSI     DDB2		// the data register that corresponds with the MOSI SPI pin of the 32UF AVR chip
#define DD_SS       DDB0		// the data register that corresponds with the SS SPI pin of the 32UF AVR chip
#define DD_SCK      DDB1		// the data register that corresponds with the SCK SPI pin of the 32UF AVR chip

/* Memory Map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17

/* Bit Mnemonics */
#define MASK_RX_DR  6
#define MASK_TX_DS  5
#define MASK_MAX_RT 4
#define EN_CRC      3
#define CRCO        2
#define PWR_UP      1
#define PRIM_RX     0
#define ENAA_P5     5
#define ENAA_P4     4
#define ENAA_P3     3
#define ENAA_P2     2
#define ENAA_P1     1
#define ENAA_P0     0
#define ERX_P5      5
#define ERX_P4      4
#define ERX_P3      3
#define ERX_P2      2
#define ERX_P1      1
#define ERX_P0      0
#define AW          0
#define ARD         4
#define ARC         0
#define PLL_LOCK    4
#define RF_DR       3
#define RF_PWR      1
#define LNA_HCURR   0        
#define RX_DR       6
#define TX_DS       5
#define MAX_RT      4
#define RX_P_NO     1
#define TX_FULL     0
#define PLOS_CNT    4
#define ARC_CNT     0
#define TX_REUSE    6
#define FIFO_FULL   5
#define TX_EMPTY    4
#define RX_FULL     1
#define RX_EMPTY    0

/* Instruction Mnemonics */
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define NOP           0xFF

// Config Options - set all options to default values
#define CONFIG_VAL (1<<EN_CRC) | (1<<MASK_TX_DS) | (1<<MASK_MAX_RT)

// private function prototypes
void SPIsetup(void);
void RFwriteReg(char writeReg, char* writeDat, int numBytes);
void RFfillTransferBuffer( char* txDat, int numBytes );
void RFreadReg(char readReg, char* retDat, int numBytes);
void RFsetRxAddr(char * recvAddr, int numBytes);
void RFflushTXBuffer(void);
void RFflushRXBuffer(void);
void RFstartReceiving(void);
void RFstopReceiving(void);
int RFRXbufferEmpty(void);
void RFreadRXFIFO( char* retDat);
int RFRXbufferFull(void);

#endif
