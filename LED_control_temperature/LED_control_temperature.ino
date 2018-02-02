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
The active LED and the temperature are shown on the LCD and on the UART.


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

// IR variables
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

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
 * 
 ************************************************************************************/
void setup() {

  // initialize the lcd 
  lcd.init();
  lcd.backlight();

  //init UART
  Serial.begin(9600); 
  
  // initialize the LED pins as an output
  pinMode(intLedRed,   OUTPUT);
  pinMode(intLedGreen, OUTPUT);
  pinMode(intLedBlue,  OUTPUT);


  // initialize the pushbutton pin as an input
  pinMode(buttonPin, INPUT);

  irrecv.enableIRIn(); // Start the IR receiver
  
  // Print the configuration
  Serial.print("Project description: \n ");
  Serial.print("Control of 3 LEDs through a button, with the current active LED displayed on an LCD \n\n");

  Serial.print("Configuration: \n");
  Serial.print("Button Pin:     ");
  Serial.println(buttonPin, DEC);
  Serial.print("Red Led Pin:    ");
  Serial.println(intLedRed, DEC);
  Serial.print("Green Led Pin:  ");
  Serial.println(intLedGreen, DEC);
  Serial.print("Blue Led Pin:   ");
  Serial.println(intLedBlue, DEC);

  Serial.print("\nStart ledactiv: ");
  Serial.println(ledactiv, DEC);


  // set RED pin on
  digitalWrite(intLedRed, HIGH);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("RED led is on!");

  // read temperature
  readTemperature();
}

/************************************************************************************
 * 
 ************************************************************************************/
void loop() {

  buttonControlLED(); // button Control
  IRControlLED();     //IR Control
  
  if (intTaskCounter == 0)
  {
    readTemperature(); // temperature read
  }
  
  intTaskCounter++;
  if (intTaskCounter > 9)
  {
    intTaskCounter = 0;
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
  //convert millivolts into flTemperature
  flTemperature = (voltage - .5) * 100;
  
  if (flOldTemperature != flTemperature)
  {

    lcd.setCursor(0, 0);
    lcd.print("Room Temp: ");
    lcd.print(flTemperature);

    Serial.print("Temperature: ");
    Serial.println(flTemperature, 6);

    Serial.print("Old temperature: ");
    Serial.println(flOldTemperature, 6);

    flOldTemperature = flTemperature;
  }

}


/************************************************************************************
 * 
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
          Serial.print("CH--IR_CONTROL_CHMINUS\n");
          ledactiv--;
      
          if (ledactiv == 0)
          {
            ledactiv = 3;
          }
          break;
        }

        case IR_CONTROL_CHPLUS: //CH++
        {
          Serial.print("IR_CONTROL_CHPLUS\n");
          ledactiv++;
      
          if (ledactiv == 4)
          {
            ledactiv = 1;
          }
          break;
        }
      
        case IR_CONTROL_BUTON1:
        {
          Serial.print("IR_CONTROL_BUTON1\n");
          ledactiv = 1;
          break;
        }
        case IR_CONTROL_BUTON2:
        {
          Serial.print("IR_CONTROL_BUTON2\n");
          ledactiv = 2;
          break;
        }
        case IR_CONTROL_BUTON3:
        {
          Serial.print("IR_CONTROL_BUTON3\n");
          ledactiv = 3;
          break;
        }

        default: // nothing here
        break;
      }
      Serial.print("ledactiv: ");
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
  
      Serial.print("ledactiv: ");
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

