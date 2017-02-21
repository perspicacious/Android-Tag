/*
Arduino Code for Pocket Pew Android App

### = code that we wrote
*/


// SOFTWARE UART SETTINGS
// ----------------------------------------------------------------------------------------------
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true  // If set to 'true' enables debug output
#define BLUEFRUIT_SWUART_RXD_PIN       9    // Required for software serial
#define BLUEFRUIT_SWUART_TXD_PIN       10   // Required for software serial
#define BLUEFRUIT_UART_CTS_PIN         11   // Required for software serial
#define BLUEFRUIT_UART_RTS_PIN         -1   // -1 if unused


// HARDWARE UART SETTINGS
// ----------------------------------------------------------------------------------------------
#ifdef Serial1    // this makes it not complain on compilation if there's no Serial1
  #define BLUEFRUIT_HWSERIAL_NAME      Serial1
#endif


// SHARED UART SETTINGS
// ----------------------------------------------------------------------------------------------
// The following sets the optional Mode pin, its recommended but not required
// ----------------------------------------------------------------------------------------------
#define BLUEFRUIT_UART_MODE_PIN        12    // Set to -1 if unused



// Our defined variables
#define IRpin_Receiver                  2 // Defines IR Receiver to pin 2 ###
#define IRpin_LED                       3 // Defines IR LED to pin 3 ###
#define colorLED                        4 // Defines color LED to pin 4 ###


#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

//__________________________________________________________________//
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

// Our non-constant variables ###
int count = 0; // This variable makes sure the reciever does not get overwhelmed. ###
int IR = 1;    // This variable is for the IR Receiver. 1 = Not hit, 0 = hit. ###
int lastIR = 1;// Helps prevent false readings

SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);


void setup(void) {

  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
   // error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
     // error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use the Pocket Pew app to connect in UART mode"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

  
  // Modes for the pins we are using ###
  pinMode(IRpin_Receiver, INPUT);  // IR Receiver, set to input ###
  pinMode(IRpin_LED, OUTPUT);      // IR LED, set to output ###
  pinMode(colorLED, OUTPUT);       // colorLED, set to output ###
  digitalWrite(IRpin_LED, LOW);    // IR LED set to no voltage on setup ###
  digitalWrite(colorLED, LOW);     // colorLED set to no voltage on setup ###
}

// Below is all our code in a loop ###
void loop(void) {

//Serial.println(digitalRead(IRpin_Receiver));

IR = digitalRead(IRpin_Receiver); // set IR to LED Receiver value ###
//Serial.print(IR);
//Serial.println(lastIR);

// Data sending to Android phone ###
// if IR and last IR = 0 then we have a hit ###
// Count needs to equal 0 also ###
if (IR == 0 && lastIR == 0)
{
  if (count == 0) 
    {
    // Send r to Serial ###
    // Serial.print("Sending: ");
    // Serial.println("r");
  
    // Send r to Android via Bluefruit ###
    ble.println("r");
  
    count = 1; // set count to 1 ###
    }
}

lastIR = IR; // Set value of IR to last IR

//Serial.print("count is: ");
//Serial.println(count); 
if (count == 350) // if count is 350 set back to 0
{                 // We are ready to send an 'r' if neccessary ###
  count = 0;  
}

if ( count != 0 ) // increment count if it is not equal to 0 ###
{
  count++;
}


// Data coming from Android phone
while ( ble.available() ) // While the bluetooth buffer is open ###
{
  char f = ble.read(); // read variable from bluetooth ###

  //Serial.print(f);
    
  if (f == 'f') // if f = 'f' ###
    {
      tone (IRpin_LED, 38000, 300); // This lights the IR LED for 300 milliseconds at 38KHz ###
      digitalWrite(colorLED, HIGH); // Lights the color LED on the clip ###
      delay(80); //delay 200 milliseconds ###
    }
  digitalWrite(colorLED, LOW); // Shuts off the color LED on the clip ###
}
delay(10);

} // end loop()
