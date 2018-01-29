/*
  LED control and temperature measurement

 Application based on:
 - 3 LEDs
 - 1 temperature sensor
 - 1 IR sensor
 - 1 IR Remote control
 - 1 button
 - 1 LCD Display

The application is used to control which of the 3 LEDs is on (only one at any given time)
and to read the exterior temperature.
The LED control is done either by button or by Remote Control.
The active LED and the temperatured are shown on the LCD and on the UART.


 The circuit:
 * LEDs attached from pin 3,4,5 to ground
 * pushbutton attached to pin 2 from +5V
 * 10K resistor attached to pin 2 from ground
 * TMP35GS sensor attached to GND, +5V and Analog channel A0.
 * IR sensor attached to GND, + 5V and pin 11.
 * 1602 LCD display attached via I2C
 * 
 * Note: on most Arduinos there is already an LED on the board
 attached to pin 13.

 */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

// IR variables
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

// constants won't change. They're used here to
// set pin numbers:
const int buttonPin = 2;    // the number of the pushbutton pin
const int ledRosu   = 3;     // the number of the LED pin
const int ledVer    = 4;
const int ledAlb    = 5;
const int sensorPin = A0;

      int ledactiv  = 1;

// variables will change:
int buttonState;     // variable for reading the pushbutton status
int buttonStateOld;  // variable for keeping previous pushbutton status


//LCD
LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x3F for a 16 chars and 2 line display

//various
int   taskCounter    = 0;
float oldTemperature = 0;
float temperature    = 1;

/************************************************************************************
 * 
 ************************************************************************************/
void setup() {

  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  Serial.begin(9600); 
  // initialize the LED pin as an output:
  pinMode(ledRosu,   OUTPUT);
  pinMode(ledVer,    OUTPUT);
  pinMode(ledAlb,    OUTPUT);


  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  // set RED on
  digitalWrite(ledRosu, HIGH);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("RED led is on!");

  readTemperature();
  irrecv.enableIRIn(); // Start the receiver
  
  // Print the configuration
  Serial.print("Project description: \n ");
  Serial.print("Control of 3 LEDs through a button, with the current active LED displayed on an LCD \n\n");

  Serial.print("Configuration: \n");
  Serial.print("Button Pin:     ");
  Serial.println(buttonPin, DEC);
  Serial.print("Red Led Pin:    ");
  Serial.println(ledRosu, DEC);
  Serial.print("Green Led Pin:  ");
  Serial.println(ledVer, DEC);
  Serial.print("Blue Led Pin:   ");
  Serial.println(ledAlb, DEC);

  Serial.print("\nStart ledactiv: ");
  Serial.println(ledactiv, DEC);

}

/************************************************************************************
 * 
 ************************************************************************************/
void loop() {

  buttonControlLED();
  IRControlLED();
  
  if (taskCounter == 0)
  {
    readTemperature();
  }
  
  taskCounter++;
  if (taskCounter > 9)
  {
    taskCounter = 0;
  }
  delay (100); // 1 second loop
}

/************************************************************************************
 * 
 ************************************************************************************/
void changebutonPin()
{
     ledactiv = ledactiv + 1;

    if (ledactiv == 4)
    {
      ledactiv = 1;
    } 
}


/************************************************************************************
 * 
 ************************************************************************************/
void readTemperature()
{
    // Read the sensor value
  int sensorVal = analogRead(sensorPin);
  // Convert reading to voltage 
  float voltage = (sensorVal/1024.0) * 5.0;
  //convert millivolts into temperature
  temperature = (voltage - .5) * 100;

  
  if (oldTemperature != temperature)
  {
    oldTemperature = temperature;
    lcd.setCursor(0, 0);
    lcd.print("Room Temp: ");
    lcd.print(temperature);
    Serial.print("Temperature:");
    Serial.println(temperature, 6);

    Serial.print("Old temperature: ");
    Serial.println(oldTemperature, 6);
  }

}


/************************************************************************************
 * 
 ************************************************************************************/
void IRControlLED()
{

   if (irrecv.decode(&results))
   {
      Serial.println(results.value, HEX);
      Serial.println(results.value, DEC);
      Serial.println(0xFFE21D, DEC);
      Serial.println(0xFFA25D, DEC);

      //CH++
      if (results.value == 0xFFE21D){
        Serial.print("CH++\n");
        ledactiv++;
    
        if (ledactiv == 4)
        {
          ledactiv = 1;
        }
      }

      //CH--
      if (results.value == 0xFFA25D){
        Serial.print("CH--\n");
        ledactiv--;
    
        if (ledactiv == 0)
        {
          ledactiv = 3;
        }
      }

      Serial.print("ledactiv: ");
      Serial.println(ledactiv, DEC);

      if (ledactiv == 1)      // RED is on
      {
         digitalWrite(ledRosu, HIGH);
         digitalWrite(ledAlb,  LOW);
         digitalWrite(ledVer,  LOW);

          lcd.setCursor(0,1);
         lcd.print("RED led is on!  ");
      }
      else if (ledactiv == 2) // WHITE is on
      {
         digitalWrite(ledAlb,  HIGH); 
         digitalWrite(ledVer,  LOW);
         digitalWrite(ledRosu, LOW);

         lcd.setCursor(0,1);
         lcd.print("BLUE led is on! ");
      }
      else if (ledactiv == 3) // GREEN is on
      {
         digitalWrite(ledVer,  HIGH);
         digitalWrite(ledRosu, LOW);
         digitalWrite(ledAlb,  LOW);

         lcd.setCursor(0,1);
         lcd.print("GREEN led is on!");
      }
     irrecv.resume(); // Receive the next value
   }
}

/************************************************************************************
 * 
 ************************************************************************************/
void buttonControlLED()
{

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  //Serial.print("buttonState: ");
  //Serial.println(buttonState, DEC);
 
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH){
   
    // update only on changed state
    if(buttonStateOld == LOW){

      //update old button state
      buttonStateOld = HIGH;
      
      ledactiv = ledactiv + 1;
  
      if (ledactiv == 4)
      {
        ledactiv = 1;
      }
  
      Serial.print("ledactiv: ");
      Serial.println(ledactiv, DEC);

      if (ledactiv == 1)      // RED is on
      {
         digitalWrite(ledRosu, HIGH);
         digitalWrite(ledAlb,  LOW);
         digitalWrite(ledVer,  LOW);

          lcd.setCursor(0,1);
         lcd.print("RED led is on!  ");
      }
      else if (ledactiv == 2) // WHITE is on
      {
         digitalWrite(ledAlb,  HIGH); 
         digitalWrite(ledVer,  LOW);
         digitalWrite(ledRosu, LOW);

         lcd.setCursor(0,1);
         lcd.print("BLUE led is on! ");
      }
      else if (ledactiv == 3) // GREEN is on
      {
         digitalWrite(ledVer,  HIGH);
         digitalWrite(ledRosu, LOW);
         digitalWrite(ledAlb,  LOW);

         lcd.setCursor(0,1);
         lcd.print("GREEN led is on!");
      }
    }
  }
  else{
   buttonStateOld = LOW;
  }
}

