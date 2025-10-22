
/*********************************************************************
*                  FUNCTIONS FOR DS1722 SENSOR                       *
*                                                                    *
**********************************************************************
*/

#include "DS1722.h"
#include "STM32L432KC_GPIO.h"


/*********************************************************************
*
*       setUpDS(char byteS)
*
*       writes to DS1722 setting
*     
*/
void setUpDS(char byteS){
  digitalWrite(SPI_CE, 1);
  spiSendReceive(0x80); 
  spiSendReceive(byteS);
  digitalWrite(SPI_CE, 0);
  }

/*********************************************************************
*
*       msbRead()
*
*       Reads the MSB
*       CONSQUENCE: MAKES THE DS1722 AUTOMATICALLY INCREMENT TO THE LSB
*     
*/
uint8_t msbRead(void){
    uint8_t msb;
    digitalWrite(SPI_CE, 1);
    spiSendReceive(0x02);        // read MSB addr
    msb = spiSendReceive(0x00);  // dummy to clock out
    digitalWrite(SPI_CE, 0);
    return msb;

  }

/*********************************************************************
*
*       lsbRead()
*
*       Reads the lsb
*         CONSQUENCE: DS1722 INCREMENTS TO REGISTERS AFTER RUNNING
*     
*/
uint8_t lsbRead(void){
    uint8_t lsb;
    digitalWrite(SPI_CE, 1);
    spiSendReceive(0x01);        // read MSB addr
    lsb = spiSendReceive(0x00);  // dummy to clock out
    digitalWrite(SPI_CE, 0);
    return lsb;
}

/*********************************************************************
*
*       configRead()
*
*       Reads the config register
*     
*/
uint8_t configRead(void){
    uint8_t config;
    digitalWrite(SPI_CE, 1);
    spiSendReceive(0x00);          // read config command
    config = spiSendReceive(0x00); // dummy to clock out config
    digitalWrite(SPI_CE, 0);
    return config;
}

/*********************************************************************
*
*       calculateTemp()
*
*       Takes in the data from the sensor and calulates the celsius equivilant
*       Returns temp, no data is adjusted
*  
*/
float calculateTemp(uint8_t MSB, uint8_t LSB) {
    int16_t rawTemp = ((MSB << 8) | LSB) >> 4;  // Combine and right-shift to 12-bit value
    // Sign-extend if negative (since 12-bit two’s complement)
    if (rawTemp & 0x0800) rawTemp |= 0xF000;    // Fill upper bits for negatives

    float temp = rawTemp * 0.0625f; //We can leave it as it were 12-bit  
    return temp;
}

