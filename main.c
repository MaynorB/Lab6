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
#include "DS1722.h"


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

  pinMode(PB6, GPIO_OUTPUT);
  
  //Timer15
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  initSPI(0b111, 0, 1); //Using very slow clock to make sure it'll work
for (int i = 0; i < 30; i++) {
  digitalWrite(SPI_CE, 1);       // select DS1722
  spiSendReceive(0x00);          // send dummy byte to generate clocks
  digitalWrite(SPI_CE, 0);       // deselect

}


  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
    // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
  }

    //SPI code here for reading temperature
    uint8_t config;
    digitalWrite(SPI_CE, 1);
    spiSendReceive(0x00);          // read config command
    config = spiSendReceive(0x00); // dummy to clock out config
    digitalWrite(SPI_CE, 0);
    printf("config: %02X\n", config);

    // // ---- Read TEMP MSB ----
    uint8_t msb;
    digitalWrite(SPI_CE, 1);
    spiSendReceive(0x02);        // read MSB addr
    msb = spiSendReceive(0x00);  // dummy to clock out
    digitalWrite(SPI_CE, 0);
    printf("MSB: %02X\n", msb);

    //// ---- Read TEMP LSB ----
    uint8_t lsb;
    digitalWrite(SPI_CE, 1);
    spiSendReceive(0x01);        // read LSB addr
    lsb = spiSendReceive(0x00);  // dummy to clock out
    digitalWrite(SPI_CE, 0);
    printf("LSB: %02X\n", lsb);
    digitalWrite(SPI_CE, 1);
    float temperature = calculateTemp(msb,  lsb);
    char tempStr[50];
    sprintf(tempStr,"TEMPERATURE IS %.2f C",temperature);
    printf("%f\n", temperature);



    // Update string with current LED state
    int led_status = updateLEDStatus(request);
    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");


    
    //// finally, transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED

    sendString(USART, "<h2>LED Status</h2>");


    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    sendString(USART, "<h2>TEMPERATURE</h2>");

    sendString(USART, "<p>");
    sendString(USART, tempStr);
    sendString(USART, "</p>");
  
    sendString(USART, webpageEnd);
  }
}