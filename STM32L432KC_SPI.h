 // STM32F401RE_SPI.h
// Header for SPI functions

#ifndef STM32L4_SPI_H
#define STM32L4_SPI_H

#include <stdint.h>
#include <stm32l432xx.h>


#define SPI_CE PA11
#define SPI_SCK PA5
#define SPI_MOSI PB5
#define SPI_MISO PB4


 /*********************************************************************
*
*       initSPI()
*
*       Sets up SPI
*       br: (0b0000 - 0b1111) sets the baud rate, fixing the SPI clock to SYSCLK / 2^(BR+1)
*       cplo: sets the clock polarity,
*             0: inactive at logic 0
*             1: inactive at logic 1
*       cpha: sets the clock phase. 
*             0: data is capturred on leading edge and changed on next edge
*             1: data is changed on leading edge and captured on next edge
*
*  
*       CONSQUENCES: PB3, PB4, PB5, PA11 WILL BE USED FOR SPI
*       PB3: SP1 CLOCK
*       PB4: SP1 MISO
*       PB5: SP1 MOSI
*       PA11: MANNUAL CHIP SELECT
*
*/
void initSPI(int br, int cpol, int cpha);

/* Transmits a character (1 byte) over SPI and returns the received character.
 *    -- send: the character to send over SPI
 *    -- return: the character received over SPI */
char spiSendReceive(char send);

#endif