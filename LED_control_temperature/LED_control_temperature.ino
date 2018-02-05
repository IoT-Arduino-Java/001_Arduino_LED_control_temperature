/*
  LED control and temperature measurement

 Application based on:
 - 3 LEDs
 - 1 temperature sensor
 - 1 Real Time Clock DS3231
 - 1 IR sensor
 - 1 IR Remote control
 - 1 button
 - 1 LCD Display

The application is used to control which of the 3 LEDs is on (only one at any given time)
and to read the exterior temperature.
The LED control is done either by button or by Remote Control.
The active LED and the temperature are shown on the LCD and on the UART.


 The circuit:
 * - LEDs attached from pin 3,4,5 to ground
 * - pushbutton attached to pin 2 from +5V
 * - 10K resistor attached to pin 2 from ground
 * - TMP35GS sensor attached to GND, +5V and Analog channel A0.
 * - IR sensor attached to GND, + 5V and pin 11.
 * - DS3231 sensor attached via I2C
 * - 1602 LCD display attached via I2C
 * 
 * Note: on most Arduinos there is already an LED on the board
 attached to pin 13.

 */
 
 /************************************************************************************
 * Includes
 ************************************************************************************/
#include <Wire.h>                //I2C library
#include <LiquidCrystal_I2C.h>   //LCD I2C library
#include <DS3231.h>              //temperature sensor library
#include <IRremote.h>            //IR remote control library


/************************************************************************************
 * Defines
 ************************************************************************************/
//IR remote control button codes
#define IR_CONTROL_CHMINUS   0xFFA25D
#define IR_CONTROL_CH        0xFF629D
#define IR_CONTROL_CHPLUS    0xFFE21D
#define IR_CONTROL_PREV      0xFF22DD
#define IR_CONTROL_NEXT      0xFF02FD
#define IR_CONTROL_PLAYPAUSE 0xFFC23D
#define IR_CONTROL_VOL1      0xFFE01F
#define IR_CONTROL_VOL2      0xFFA857
#define IR_CONTROL_EQ        0xFF906F
#define IR_CONTROL_BUTON0    0xFF6897
#define IR_CONTROL_BUTON100  0xFF9867
#define IR_CONTROL_BUTON200  0xFFB04F
#define IR_CONTROL_BUTON1    0xFF30CF
#define IR_CONTROL_BUTON2    0xFF18E7
#define IR_CONTROL_BUTON3    0xFF7A85
#define IR_CONTROL_BUTON4    0xFF10EF
#define IR_CONTROL_BUTON5    0xFF38C7
#define IR_CONTROL_BUTON6    0xFF5AA5
#define IR_CONTROL_BUTON7    0xFF42BD
#define IR_CONTROL_BUTON8    0xFF4AB5
#define IR_CONTROL_BUTON9    0xFF52AD


/************************************************************************************
 * Variables
 ************************************************************************************/
// IR variables
int RECV_PIN = 6;
IRrecv irrecv(RECV_PIN);
decode_results results;

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// constants won't change. They're used here to
// set pin numbers:
const int buttonPin   = 2;    // the number of the pushbutton pin
const int intLedRed   = 3;    // the number of the LED pin
const int intLedGreen = 4;
const int intLedBlue  = 5;
const int sensorPin   = A0;

      int ledactiv  = 1;

// variables will change:
int intButtonState;     // variable for reading the pushbutton status
int intButtonStateOld;  // variable for keeping previous pushbutton status


//LCD
LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x3F for a 16 chars and 2 line display

//various
int   intTaskCounter   = 0;
float flTemperature    = 1;
float flOldTemperature = 0;


/************************************************************************************
 * Functions
 ************************************************************************************/
 
/************************************************************************************
 *  Name: setup()
 *  Description: Startup code which runs before the main loop
 ************************************************************************************/
void setup() {

  // initialize the lcd 
  lcd.init();
  lcd.backlight();

  //init UART
  Serial.begin(9600); 
  
  // initialize the LED pins as output
  pinMode(intLedRed,   OUTPUT);
  pinMode(intLedGreen, OUTPUT);
  pinMode(intLedBlue,  OUTPUT);


  // initialize the pushbutton pin as an input
  pinMode(buttonPin, INPUT);

  irrecv.enableIRIn(); // Start the IR receiver
  
  // Print the configuration
  Serial.print("Project description: \n");
  Serial.print("1. Control of 3 LEDs through a button or an IR remote control, with the current active LED displayed on an LCD\n");
  Serial.print("2. Real time clock monitioring with date and time displayed on LCD based on remote control selection (to be implemented)\n\n");

  Serial.print("**************************\n");
  Serial.print("* Configuration: \n");
  Serial.print("**************************\n\n");
  Serial.print("Pin Button:     ");
  Serial.println(buttonPin, DEC);
  Serial.print("Pin Red Led:    ");
  Serial.println(intLedRed, DEC);
  Serial.print("Pin Green Led:  ");
  Serial.println(intLedGreen, DEC);
  Serial.print("Pin Blue Led:   ");
  Serial.println(intLedBlue, DEC);
  Serial.print("Pin IR LED:     ");
  Serial.println(RECV_PIN, DEC);
  Serial.print("Pin Temp sens:  ");
  Serial.println(sensorPin, DEC);
  Serial.print("\n**************************\n");
  
  Serial.print("\n\nActive LED: ");
  Serial.println(ledactiv, DEC);


  // set RED pin on
  digitalWrite(intLedRed, HIGH);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("RED led is on!");


  // set temperature analog channel as input
  pinMode(sensorPin, INPUT);
  // Initialize the rtc object
  rtc.begin();
  // read temperature
  readTemperature();

}

/************************************************************************************
 * Name: loop()
 * Description: Main application loop function
 ************************************************************************************/
void loop() {

  // 100 ms operations
  buttonControlLED();       // button Control
  IRControlLED();           // IR Control

  // 1 second operations
  if (intTaskCounter == 0)
  {
    readTemperature();      // read temperature
    readClock();            // read clock 
  }

  //increase task counter
  intTaskCounter++;
  
  if (intTaskCounter > 9)
  {
    intTaskCounter = 0;
  }
  
  delay (100); // 100 ms loop
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
 * Name readTemperature()
 * Description: reads the temperature for the DS3231 sensor
 ************************************************************************************/
void readTemperature()
{
  // Read the sensor value
  int sensorVal = analogRead(sensorPin);
  // Convert reading to voltage 
  float voltage = (sensorVal/1024.0) * 5.0;
  //convert millivolts into flTemperature
  flTemperature = (voltage - .5) * 100;
  
  if (flOldTemperature != flTemperature)
  {

    lcd.setCursor(0, 0);
    lcd.print("Room Temp: ");
    lcd.print(flTemperature);

    Serial.print("\nTemperature: ");
    Serial.println(flTemperature, 6);

    Serial.print("Old temperature: ");
    Serial.println(flOldTemperature, 6);
    
    //read Clock
    //readClock();
    
    flOldTemperature = flTemperature;
  }

}


/************************************************************************************
 * Name IRControlLED()
 * Description: processes the IR commands
 ************************************************************************************/
void IRControlLED()
{

   // decode IR code
   if (irrecv.decode(&results))
   {
      //Serial.println(results.value, HEX);

      switch (results.value)
      {
        case IR_CONTROL_CHMINUS: //CH--
        {
          Serial.print("\nIR_CONTROL_CHMINUS\n");
          ledactiv--;
      
          if (ledactiv == 0)
          {
            ledactiv = 3;
          }
          break;
        }

        case IR_CONTROL_CHPLUS: //CH++
        {
          Serial.print("\nIR_CONTROL_CHPLUS\n");
          ledactiv++;
      
          if (ledactiv == 4)
          {
            ledactiv = 1;
          }
          break;
        }
      
        case IR_CONTROL_BUTON1:
        {
          Serial.print("\nIR_CONTROL_BUTON1\n");
          ledactiv = 1;
          break;
        }
        case IR_CONTROL_BUTON2:
        {
          Serial.print("\nIR_CONTROL_BUTON2\n");
          ledactiv = 2;
          break;
        }
        case IR_CONTROL_BUTON3:
        {
          Serial.print("\nIR_CONTROL_BUTON3\n");
          ledactiv = 3;
          break;
        }

        default: // nothing here
        break;
      }

      Serial.print("\nActive LED: ");
      Serial.println(ledactiv, DEC);

      if (ledactiv == 1)      // RED is on
      {
         digitalWrite(intLedRed,   HIGH);
         digitalWrite(intLedBlue,  LOW);
         digitalWrite(intLedGreen, LOW);

         lcd.setCursor(0,1);
         lcd.print("RED led is on!  ");
      }
      else if (ledactiv == 2) // WHITE is on
      {
         digitalWrite(intLedBlue,  HIGH); 
         digitalWrite(intLedGreen, LOW);
         digitalWrite(intLedRed,   LOW);

         lcd.setCursor(0,1);
         lcd.print("BLUE led is on! ");
      }
      else if (ledactiv == 3) // GREEN is on
      {
         digitalWrite(intLedGreen, HIGH);
         digitalWrite(intLedRed,   LOW);
         digitalWrite(intLedBlue,  LOW);

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
  intButtonState = digitalRead(buttonPin);

  //Serial.print("intButtonState: ");
  //Serial.println(intButtonState, DEC);
 
  // check if the pushbutton is pressed.
  // if it is, the intButtonState is HIGH:
  if (intButtonState == HIGH){
   
    // update only on changed state
    if(intButtonStateOld == LOW){

      //update old button state
      intButtonStateOld = HIGH;
      
      ledactiv = ledactiv + 1;
  
      if (ledactiv == 4)
      {
        ledactiv = 1;
      }
  
      Serial.print("\nledactiv: ");
      Serial.println(ledactiv, DEC);
      

      if (ledactiv == 1)      // RED is on
      {
         digitalWrite(intLedRed,   HIGH);
         digitalWrite(intLedBlue,  LOW);
         digitalWrite(intLedGreen, LOW);

         lcd.setCursor(0,1);
         lcd.print("RED led is on!  ");
      }
      else if (ledactiv == 2) // WHITE is on
      {
         digitalWrite(intLedBlue,  HIGH); 
         digitalWrite(intLedGreen, LOW);
         digitalWrite(intLedRed,   LOW);

         lcd.setCursor(0,1);
         lcd.print("BLUE led is on! ");
      }
      else if (ledactiv == 3) // GREEN is on
      {
         digitalWrite(intLedGreen,  HIGH);
         digitalWrite(intLedRed,    LOW);
         digitalWrite(intLedBlue,   LOW);

         lcd.setCursor(0,1);
         lcd.print("GREEN led is on!");
      }
    }
  }
  else{
   intButtonStateOld = LOW;
  }
}


/************************************************************************************
 * 
 ************************************************************************************/
void readClock()
{
  // Send Day-of-Week
  Serial.print("\nDay: ");
  Serial.println(rtc.getDOWStr());
  
  // Send Date
  Serial.print("Date: ");
  Serial.println(rtc.getDateStr());
  
  // Send Time
  Serial.print("Time: ");  
  Serial.println(rtc.getTimeStr());
  
  // Wait one second before repeating :)
  //delay (1000);
}

