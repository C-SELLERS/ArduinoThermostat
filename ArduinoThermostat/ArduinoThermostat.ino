
/*
CSS 427 UWB
CODE FOR ARDUINO THERMOSTAT
DHT SENSOR
*/


#include <dht_nonblocking.h>
#include <IRremote.h>
#include <LiquidCrystal.h>


//Definition for remote functions
#define POWER         0xFFA25D
#define FUNC          0xFFE21D
#define VOLUP         0xFF629D
#define REWIND        0xFF22DD
#define PAUSE         0xFF02FD
#define FASTFOR       0xFFC23D
#define DOWN          0xFFE01F
#define VOLDOWN       0xFFA857
#define UP            0xFF906F
#define EQ            0xFF9867
#define STREPT        0xFFB04F
#define ZERO          0xFF6897
#define ONE           0xFF30CF
#define TWO           0xFF18E7
#define THREE         0xFF7A85
#define FOUR          0xFF10EF
#define FIVE          0xFF38C7
#define SIX           0xFF5AA5
#define SEVEN         0xFF42BD
#define EIGHT         0xFF4AB5
#define NINE          0xFF52AD
#define HOLD          0xFFFFFFFF

//Definition for volume button mapping
#define CONTROLTEMP   0
#define CONTROLHUMID  1
#define CONTROLFAN    2

// Definition for fan range
#define MAXRES        150
#define MINRES        85
#define OFF           0
#define RESSTEP       32

//Definition for operating modes
#define MANUAL        true
#define AUTO          false

//These are the pin outs to the arduino from the H-Bridge IC 
#define ENABLE        5
#define DIRA          6
#define DIRB          4
 
//Initialize LCD Screen
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
//                RS E  D4 D5  D6  D7

//Initialize DHT Sensor variables
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
int ReadTime = 2000; 
unsigned long Checkpoint = 0;
unsigned long Time = millis();

//Sensor Variables
float temperature;
float humidity;

//Variables for settings
int setTemp = 30;
int setHumid = 45;
int curTemp = 0;
int curHum = 0;
int resolution = (MAXRES + MINRES) / 2;
bool changes = true; //true for initial bootup
bool on = false; //Current signal to fan
bool mode = MANUAL; //System's current mode
int volButtonMapping = CONTROLTEMP; //Indicate which threshold VOL buttons will change

//Initialize the remote control variables
int receiverPin = 3;
IRrecv irrecv(receiverPin);
decode_results results;      

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  //H-Bridge pins
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
  
  //Start the receiver
  irrecv.enableIRIn(); 
}

void loop(){
  
  //This is to move the fan in one direction ***
    digitalWrite(DIRA,HIGH); //one way
    digitalWrite(DIRB,LOW);
   
  //Read the temp and humidity every 2 seconds
    Time = millis();
    if(Time - Checkpoint > ReadTime){
        Checkpoint = Time;
        measureConditions();
    }

    //If there are changes check conditions and update screen
    if(changes){
        //Relay condition to fan
        if(mode == AUTO) {
        if(temperature > setTemp + 6 || humidity > setHumid + 12)
        {
          resolution = MAXRES;
        }
        else if(temperature > setTemp + 3 || humidity > setHumid + 6)
        {
          resolution = (MAXRES + MINRES) / 2;
        }
        else if(temperature > setTemp || humidity > setHumid)
        {
          resolution = MINRES;
        }
        else 
        {
          resolution = OFF;
        }
        turn();
      } else if(mode == MANUAL){
        turn();
      }

        updateScreen();
        changes = false;
    }

  
    //Remote commands 
    if (irrecv.decode(&results)){
      changes = true; 
      ControlFunction(results);
      irrecv.resume(); // receive the next value
    }

  delay(100);
}



void printThresholdChange() {
  lcd.clear();
  lcd.setCursor(0, 0);
  switch(volButtonMapping) {
    case CONTROLTEMP:
      lcd.print("IN AUTO MODE, FAN ON AT ");
      lcd.setCursor(0,1);
      lcd.print(setTemp);
      lcd.print("C");
      break;
    case CONTROLHUMID:
      lcd.print("IN AUTO MODE, FAN ON AT ");
      lcd.setCursor(0,1);
      lcd.print(setHumid);
      lcd.print("% HUMIDITY");
      break;
    case CONTROLFAN:
      lcd.print("IN MANUAL MODE, FAN ON AT ");
      lcd.setCursor(0,1);
      lcd.print((resolution-MINRES) / (MAXRES-MINRES));
      lcd.print("% POWER");
      break;
  }
  delay(2000);
}

//YOU CAN CONTROL THE SPEED OF THE FAN BY MANIPULATING THE 2ND PARAMETER IN ANALOG WRITE
//THIS POWER/VALUE HAS A RANGE FROM 0-255 (255 FULL POWER 0 OFF); 
void turn()
{
   analogWrite(ENABLE, resolution);
   on = true;
}

//Void for changing screen if changes occur
void updateScreen(){
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("CUR");
    
    lcd.setCursor(4, 0);
    lcd.print((int)temperature);
    lcd.print("C");

    lcd.setCursor(9,0);
    lcd.print((int)humidity);
    lcd.print("%");

    lcd.setCursor(14,0);
    if(on) { 
        lcd.print("+");
    } else {
        lcd.print("-");
    }

    lcd.setCursor(0,1);
  	 lcd.print("SET");

    lcd.setCursor(4, 1);
    lcd.print(setTemp);
    lcd.print("C");
  
    lcd.setCursor(9,1);
    lcd.print(setHumid);
    lcd.print("%");
}

void measureConditions(){
    curTemp = (int)temperature;
    curHum = (int)humidity;

    //workaround, forces it to query until it gets a reading :/
    while(!dht_sensor.measure(&temperature, &humidity)){}

    if( curTemp != (int)temperature || curHum != (int)humidity){
        curTemp = (int)temperature;
        curHum = (int)humidity;
        changes = true;
    }
    
}

void ControlFunction(decode_results results){
  switch(results.value){

        //Switch the mode that the device is in
        case POWER:
          if(mode == AUTO) {
              mode = MANUAL;
              // In manual mode, volume buttons control fan strength
              volButtonMapping = CONTROLFAN;
              resolution = (MINRES + MAXRES) / 2;
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("MODE: MANUAL");
              lcd.setCursor(0, 1);
              lcd.print("CONTROLING: FAN");
           }
           //Manual goes to auto
           else {
              mode = AUTO;
              // In auto mode, volume buttons control temp threshold or humidity % threshold
              volButtonMapping = CONTROLTEMP;
              resolution = MINRES;
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("MODE: AUTO");
              lcd.setCursor(0, 1);
              lcd.print("CONTROLING: TEMP");
            }
            delay(1000); //Leave message for a second
          }
          break;
        
        //Raise the threshold indicated by volButtonMapping
        case VOLUP:
          switch(volButtonMapping){
            case CONTROLTEMP:
              setTemp++;
              break;
            case CONTROLHUMID:
              setHumid = min(100, setHumid+1);
              break;
            case CONTROLFAN:
              resolution = min(MAXRES, resolution + RESSTEP);
              turn();
              break;
          }
          //printThresholdChange(); WILL BE REMOVED CURRENT USE SCREEN FOR SHOWING CURRENT SETTTINGS
          break;

        //Lower the threshold indicated by volButtonMapping
        case VOLDOWN:
          switch(volButtonMapping){
            case CONTROLTEMP:
              setTemp--;
              break;
            case CONTROLHUMID:
              setHumid--;
              break;
            case CONTROLFAN:
              resolution = max(MINRES, resolution - RESSTEP);
              turn();
              break;
          }
          //printThresholdChange(); WILL BE REMOVED CURRENT USE SCREEN FOR SHOWING CURRENT SETTTINGS
          break;
          
        case FUNC:
          // In auto mode, vol buttons can control temp threshold or humidity % threshold
          if(mode == AUTO) {
            volButtonMapping=(volButtonMapping+1)%2;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("VOL BUTTONS CONTROL ");
            lcd.setCursor(0, 1);
            switch(volButtonMapping){
              case CONTROLTEMP:
                lcd.print("DESIRED TEMP (AUTO)");
                break;
              case CONTROLHUMID:
                lcd.print("DESIRED HUMID% (AUTO)");
                break;
            }
            delay(2000);
          }
          break;
        
        case ONE: 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("HELLLLOO");
          delay(5000);
          break;
        
        case TWO:
          lcd.clear(); 
          lcd.setCursor(0,0); 
          lcd.print("Give Us"); 
          delay(2000); 
          lcd.clear(); 
          lcd.setCursor(0,0); 
          lcd.print("A Pass");
          delay(1000);
          break;       
        
        //Maybe implement later?
        case HOLD: 
          break;
        
        default: 
          break;
  }
}
