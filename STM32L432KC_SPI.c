// STM32L432KC_SPI.c
// Maynor
// mbacitzep@hmc.edu
//UNITALIZE SPI
#include "STM32L432KC.h"
#include "STM32L432KC_SPI.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_RCC.h"

 /*********************************************************************
*
*       initSPI()
*
*       Sets up SPI
*       br (0b0000 - 0b1111) sets the baud rate, fixing the SPI clock to SYSCLK / 2^(BR+1)
*       cplo sets the clock polarity: 0 for low inactive, 1 for high inactive
*       cpha sets the clock phase. 0: data is capturred on leading edge and changed on next edge
*       1: data is changed on leading edge and captured on next edge
*
*  
*       CONSQUENCES: PB3, PB4, PB5, PA11 WILL BE USED FOR SPI
*       PB3: SP1 CLOCK
*       PB4: SP1 MISO
*       PB5: SP1 MOSI
*       PA11: MANNUAL CHIP SELECT
*
*/
void initSPI(int br, int cpol, int cpha) {
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN); // Turn on GPIOA and GPIOB clock domains (GPIOAEN and GPIOBEN bits in AHB1ENR)
    
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // Turn on SPI1 clock domain 

    // Initially assigning SPI pins, THESE PIN'S AF ARE ON SP1
    pinMode(SPI_SCK, GPIO_ALT); // SPI1_SCK PB3
    pinMode(SPI_MISO, GPIO_ALT); // SPI1_MISO PB4
    pinMode(SPI_MOSI, GPIO_ALT); // SPI1_MOSI PB5
    pinMode(SPI_CE, GPIO_OUTPUT); //  Manual CS PA11

    
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED3); // Set output speed type to high for SCK

    // Set to AF05 for SPI alternate functions
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL3, 5);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL4, 5);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 5);
    
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br); // Set baud rate divider

    SPI1->CR1 |= (SPI_CR1_MSTR); //Selects as Master
    SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_LSBFIRST | SPI_CR1_SSM); //Clears out the bits in case things went wrong on reset
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha); //Sets Clock phase
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol); //Sets Clock Polarity
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111); //Sets data transfer size: 8 bits or 1 byte
    
    //SPI1->CR2 |= (SPI_CR2_FRXTH | SPI_CR2_SSOE); //Lowkey killing this in general acutally

    SPI1->CR1 |= (SPI_CR1_SPE); // Enable SPI
}

/* Transmits a character (1 byte) over SPI and returns the received character.
 *    -- send: the character to send over SPI
 *    -- return: the character received over SPI */
char spiSendReceive(char send) {
    while(!(SPI1->SR & SPI_SR_TXE)); // Wait until the transmit buffer is empty
    *(volatile char *) (&SPI1->DR) = send; // Transmit the character over SPI
    while(!(SPI1->SR & SPI_SR_RXNE)); // Wait until data has been received
    char rec = (volatile char) SPI1->DR;
    return rec; // Return received character
}