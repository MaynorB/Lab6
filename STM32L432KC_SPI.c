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
*       initSPI(int br, int cpol, int cpha)
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

    // 1. Enable peripheral clocks
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN); // Enable GPIOA and GPIOB clocks
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;                           // Enable SPI1 clock


    // 2. Configure GPIO pins for SPI1
    //    SPI1 on PB3 = SCK, PB4 = MISO, PB5 = MOSI (AF5)

    // PB3 (SCK)
    pinMode(PB3, GPIO_ALT);                                       // Set PB3 to alternate function mode
    GPIOB->OSPEEDR |= _VAL2FLD(GPIO_OSPEEDR_OSPEED3, 0b11);       // High speed for fast clock edges
    GPIOB->AFR[0] &= ~_VAL2FLD(GPIO_AFRL_AFSEL3, 0xF);            // Clear AF bits
    GPIOB->AFR[0] |=  _VAL2FLD(GPIO_AFRL_AFSEL3, 5);              // Set AF5 (SPI1)

    // PB4 (MISO) 
    pinMode(PB4, GPIO_ALT);                                       // Set PB4 to alternate function mode
    GPIOB->OSPEEDR |= _VAL2FLD(GPIO_OSPEEDR_OSPEED4, 0b11);       // High speed
    GPIOB->AFR[0] &= ~_VAL2FLD(GPIO_AFRL_AFSEL4, 0xF);            // Clear AF bits
    GPIOB->AFR[0] |=  _VAL2FLD(GPIO_AFRL_AFSEL4, 5);              // Set AF5 (SPI1)

    //PB5 (MOSI)
    pinMode(PB5, GPIO_ALT);                                       // Set PB5 to alternate function mode
    GPIOB->OSPEEDR |= _VAL2FLD(GPIO_OSPEEDR_OSPEED5, 0b11);       // High speed
    GPIOB->AFR[0] &= ~_VAL2FLD(GPIO_AFRL_AFSEL5, 0xF);            // Clear AF bits
    GPIOB->AFR[0] |=  _VAL2FLD(GPIO_AFRL_AFSEL5, 5);              // Set AF5 (SPI1)

    // 3. Configure manual Chip Select (NSS) pin
    pinMode(PA11, GPIO_OUTPUT);                                   // PA11 used as manual chip select
    digitalWrite(PA11, 0);                                        // Set HIGH (slave deselected by default)


    // 4. Configure SPI1 peripheral registers

    SPI1->CR1 = 0;                                                // Reset control register 1
    SPI1->CR2 = 0;                                                // Reset control register 2

    // --- Step 2a: Configure baud rate (BR[2:0]) ---
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);                        // Set clock divider (f_PCLK / 2^BR)

    // --- Step 2b: Clock polarity and phase ---
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);                    // CPOL: 0 = idle low, 1 = idle high
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);                    // CPHA: 0 = first edge, 1 = second edge

    // --- Step 2c: Full-duplex master mode ---
    SPI1->CR1 |= SPI_CR1_MSTR;                                    // Set master mode
    //SPI1->CR1 &= ~(SPI_CR1_RXONLY | SPI_CR1_BIDIMODE);            // Ensure full-duplex mode (2-line)

    // --- Step 2d: Data order ---
    SPI1->CR1 &= ~SPI_CR1_LSBFIRST;                               // MSB first

    // --- Step 2f: Slave management ---
    //SPI1->CR1 &= ~SPI_CR1_SSM;                                    // Disable software NSS management
    //SPI1->CR1 &= ~SPI_CR1_SSI;                                    // Use hardware NSS control

    // --- Step 3a: Data size ---
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111);                    // 8-bit data frame

    // --- Step 3b: Enable SSOE (NSS output enable) ---
   SPI1->CR2 |= SPI_CR2_SSOE;                                    // NSS driven automatically when SPE=1

    // --- Step 3e: RX FIFO threshold ---
    SPI1->CR2 |= SPI_CR2_FRXTH;                                   // 8-bit data alignment for RX

    // ==========================================================
    // 5. Enable SPI1 peripheral
    // ==========================================================
    SPI1->CR1 |= SPI_CR1_SPE;                                     // Enable SPI
}
 /*********************************************************************
*
*       spiSendReceive(char send)
*
*       sends a character though SPI and returns char recived by DS1722
*       send: byte we are sending over
*       return: recieved byte
*
*/
char spiSendReceive(char send) {
    // Wait until transmit buffer is empty
    while (!(SPI1->SR & SPI_SR_TXE));

    // Start transmission by writing to data register
    *(volatile uint8_t *)(&SPI1->DR) = (uint8_t)send;

    // Wait until a byte is received
    while (!(SPI1->SR & SPI_SR_RXNE));

    // Read received data
    uint8_t rec = (uint8_t)SPI1->DR;

    // Wait until SPI is not busy before returning
    while (SPI1->SR & SPI_SR_BSY);

    return (char)rec;
}