/*
File: Lab_6_JHB.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";
char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}
/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////


// Function used by printf to send characters to the laptop
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}


int main(void) {
  configureFlash();
  configureClock();

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(PB3, GPIO_OUTPUT);
  
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  initSPI(0b010, 0, 0); 

  //Setting it to 12 bit resolution
  digitalWrite(SPI_CE, 0);                 // Select the DS1722
  spiSendReceive(0x80);                    // Command: write to config register
  spiSendReceive(0xC0);                    // Data: 12-bit continuous
  digitalWrite(SPI_CE, 1);                 // Deselect


  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    //while(inString(request, "\n") == -1) {
    //  // Wait for a complete request to be transmitted before processing
    //  while(!(USART->ISR & USART_ISR_RXNE));
    //  request[charIndex++] = readChar(USART);
    //}

    //SPI code here for reading temperature
    digitalWrite(SPI_CE, 0);              // Pull CS low to start the read
    spiSendReceive(0x02);                 // 0x02 tells us to start at the most significant bit
    uint8_t msb = spiSendReceive(0x00);   // Read MSB
    uint8_t lsb = spiSendReceive(0x00);   // Read LSB; it automatically decrements
    digitalWrite(SPI_CE, 1);              // Pull CS high to end the read
  
    //Convert raw data
    int16_t rawTemp = (msb << 8); //msb is shifted 8 bits as that is how many are representing integer, plus MSB is supposed to be the 8 sig digs
    rawTemp = rawTemp | lsb; //OR beacuse some bits are 0, meaning we don't want them.
    float temperature = ((uint16_t) rawTemp) * 0.0625;  // 12-bit mode = 1 LSB = 1/16 °C = raw / 256
    printf("TEMP IS: %f!\n", temperature);

    // Update string with current LED state
  
    //int led_status = updateLEDStatus(request);

    //char ledStatusStr[20];
    //if (led_status == 1)
    //  sprintf(ledStatusStr,"LED is on!");
    //else if (led_status == 0)
    //  sprintf(ledStatusStr,"LED is off!");

    //// finally, transmit the webpage over UART
    //sendString(USART, webpageStart); // webpage header code
    //sendString(USART, ledStr); // button for controlling LED

    //sendString(USART, "<h2>LED Status</h2>");


    //sendString(USART, "<p>");
    //sendString(USART, ledStatusStr);
    //sendString(USART, "</p>");

  
    //sendString(USART, webpageEnd);
  }
}