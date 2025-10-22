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
*       PA5: SP1 CLOCK
*       PB4: SP1 MISO
*       PB5: SP1 MOSI
*       PA11: MANNUAL CHIP SELECT
*
*/
void initSPI(int br, int cpol, int cpha) {
    // --- Enable GPIO and SPI1 clocks ---
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // --- Configure SPI pins ---
    // PA5 = SCK (AF5)
    pinMode(PA5, GPIO_ALT);
    GPIOA->OSPEEDR |= _VAL2FLD(GPIO_OSPEEDR_OSPEED5, 0b11);  // High speed
    GPIOA->AFR[0] &= ~_VAL2FLD(GPIO_AFRL_AFSEL5, 0xF);
    GPIOA->AFR[0] |=  _VAL2FLD(GPIO_AFRL_AFSEL5, 5);         // AF5 = SPI1

    // PB4 = MISO (AF5)
    pinMode(PB4, GPIO_ALT);
    GPIOB->OSPEEDR |= _VAL2FLD(GPIO_OSPEEDR_OSPEED4, 0b11);
    GPIOB->AFR[0] &= ~_VAL2FLD(GPIO_AFRL_AFSEL4, 0xF);
    GPIOB->AFR[0] |=  _VAL2FLD(GPIO_AFRL_AFSEL4, 5);         // AF5 = SPI1

    // PB5 = MOSI (AF5)
    pinMode(PB5, GPIO_ALT);
    GPIOB->OSPEEDR |= _VAL2FLD(GPIO_OSPEEDR_OSPEED5, 0b11);
    GPIOB->AFR[0] &= ~_VAL2FLD(GPIO_AFRL_AFSEL5, 0xF);
    GPIOB->AFR[0] |=  _VAL2FLD(GPIO_AFRL_AFSEL5, 5);         // AF5 = SPI1

    // Manual chip select (PA11)
    pinMode(PA11, GPIO_OUTPUT);
    digitalWrite(PA11, 1); // Deselect by default

    // --- SPI1 configuration ---
    SPI1->CR1 = 0;  // Reset CR1
    SPI1->CR2 = 0;  // Reset CR2

    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);     // Baud rate divider
    SPI1->CR1 |= SPI_CR1_MSTR;                 // Master mode
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);
    SPI1->CR1 &= ~(SPI_CR1_LSBFIRST | SPI_CR1_SSM); // MSB first, no software slave mgmt

    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111); // 8-bit data frame
    SPI1->CR2 |= (SPI_CR2_FRXTH | SPI_CR2_SSOE);

    // --- Enable SPI ---
    SPI1->CR1 |= SPI_CR1_SPE;
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