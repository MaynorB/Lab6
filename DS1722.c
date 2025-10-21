
/*********************************************************************
*                  FUNCTIONS FOR DS1722 SENSOR                       *
*                                                                    *
**********************************************************************
*/

#include "DS1722.h"

/*********************************************************************
*
*       calculateTemp()
*
*       Takes in the data from the sensor and calulates the celsius equivilant
*       Returns temp, no data is adjusted
*  
*/
float calculateTemp(uint8_t MSB, uint8_t LSB){
  float temp;
  //Convert raw data
  int16_t rawTemp = (MSB << 4) | LSB; //msb is shifted 8 bits as that is how many are representing integer, plus MSB is supposed to be the 8 sig digs
  temp = (rawTemp) * 0.0625;  // 12-bit mode = 1 LSB = 1/16 °C = raw / 256
  return temp;
}