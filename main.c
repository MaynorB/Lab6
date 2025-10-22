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

void setUp(void){
  configureFlash();
  configureClock();
  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(PB6, GPIO_OUTPUT);
  
  //Timer15
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);


}

// Function used by printf to send characters to the laptop
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}


int main(void) {

  setUp();
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);
  initSPI(0b111, 0, 1); //Using very slow clock to make sure it'll work
  setUpDS(0XE4); // Sets up as 10 bit resolution

  volatile uint8_t config;
  volatile uint8_t msb;
  volatile uint8_t lsb;


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

    
    config = configRead();
    msb = msbRead();
    lsb = lsbRead();
    printf("config: %02x\n", config);
    printf("MSB: %02x\n", msb);
    printf("LSB: %02x\n", lsb);


    float temperature = calculateTemp(msb,  lsb);
    char tempStr[50];
    sprintf(tempStr,"TEMPERATURE IS %.4f C",temperature);
    printf("%f\n", temperature);


    char maxStr[50];
    char minStr[50];
    float max = calculateTemp(0x19,  0x10);
    float min = calculateTemp(0xF5,  0xE0);
    sprintf(maxStr,"MAX TEMP %.4f C",max);
    sprintf(minStr,"MIN TEMP IS %.4f C",min);
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

    sendString(USART, "<p>");
    sendString(USART, maxStr);
    sendString(USART, "</p>");

    sendString(USART, "<p>");
    sendString(USART, minStr);
    sendString(USART, "</p>");
  
    sendString(USART, webpageEnd);
  }
}