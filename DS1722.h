// DS1722.h
// Maynor Bac Itzep
// mbacitzep@hmc.edu
// Oct 21, 2025
// THIS FILE HOLDS THE FUNCTION to CALCULATE TEMPERATURE WITH THE RAW DATA PROVIDED
#ifndef DS1722_H
#define DS1722_H

#include <stdint.h>
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

float calculateTemp(uint8_t MSB, uint8_t LSB);

#endif