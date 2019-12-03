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
#define MINRES        90
#define OFF           0
#define RESSTEP       15

// Defining Default TEMP, HUMIDITY, RES
#define DEFAULTTEMP   30
#define DEFAULTHUMID  45
#define DEFUALTRES    120
#define DEFAULTINC    5

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
int setTemp = DEFAULTTEMP;
int setHumid = DEFAULTHUMID;
int curTemp = 0;
int curHum = 0;
int resolution = 0;
int increment = DEFAULTINC;
bool changes = true; //true for initial bootup
bool mode = AUTO; //System's current mode
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

  //H-Bridge Direction
  digitalWrite(DIRA,HIGH); //one way
  digitalWrite(DIRB,LOW);
  
  //Start the receiver
  irrecv.enableIRIn(); 
}

void loop(){
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
        if(temperature > setTemp + 4 || humidity > setHumid + 12) {
          resolution = MAXRES;
        }
        else if(temperature > setTemp + 3 || humidity > setHumid + 9){
          resolution = ((MAXRES + MINRES) * 3) / 4;
        }
        else if(temperature > setTemp + 2 || humidity > setHumid + 6){
          resolution = (MAXRES + MINRES) / 2;
        }
        else if(temperature > setTemp + 1 || humidity > setHumid + 3){
          resolution = (MAXRES + MINRES) / 4;
        }
        else if(temperature > setTemp || humidity > setHumid){
          resolution = MINRES;
        } else {
          resolution = OFF;
        }
      } 
      // regardless do this:
      analogWrite(ENABLE, resolution);
      
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
    if(mode == MANUAL){
      lcd.print("M");
    } else {
      lcd.print("A");
    }

    lcd.setCursor(0,1);
    lcd.print("SET");

    lcd.setCursor(4, 1);
    lcd.print(setTemp);
    lcd.print("C");
  
    lcd.setCursor(9,1);
    lcd.print(setHumid);
    lcd.print("%");

    //PRINT CURRENT FAN SPEED
    lcd.setCursor(14,1);
      if(resolution == OFF){
      lcd.print("0");
    } else if (resolution == MINRES) {
      lcd.print("L");
    } else if (resolution == MAXRES) {
      lcd.print("H");
    } else if (resolution < DEFAULTRES){
      lcd.print("ML");
    } else if (resolution > DEFAULTRES){
      lcd.print("MH");
    } else {
      lcd.print("M");
    } 

  switch(volButtonMapping){
      case CONTROLTEMP:
        lcd.setCursor(3, 1);
        break;
      case CONTROLHUMID:
        lcd.setCursor(8, 1);
        break;
      case CONTROLFAN:
        lcd.setCursor(13, 1);
        break;
    }
    lcd.print("*");

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
      case FUNC:
        if(mode == AUTO) {
            mode = MANUAL;
            // In manual mode, volume buttons control fan strength
            volButtonMapping = CONTROLFAN;
            resolution = (MINRES + MAXRES) / 2;
         }
         //Manual goes to auto
         else {
            mode = AUTO;
            // In auto mode, volume buttons control temp threshold or humidity % threshold
            volButtonMapping = CONTROLTEMP;
            resolution = MINRES;
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
            analogWrite(ENABLE, resolution);
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
            analogWrite(ENABLE, resolution);
            break;
        }
        break;
  
      case FASTFOR:
        switch(volButtonMapping){
          case CONTROLTEMP:
            setTemp += increment;
            break;
          case CONTROLHUMID:
            setHumid += increment;
            break;
          case CONTROLFAN:
            resolution = min(MAXRES, resolution + (RESSTEP * 2));
            analogWrite(ENABLE, resolution);
            break;
        }
        break;
  
      case REWIND:
        switch(volButtonMapping){
          case CONTROLTEMP:
            setTemp -= increment;
            break;
          case CONTROLHUMID:
            setHumid -= increment;
            break;
          case CONTROLFAN:
            resolution = max(MINRES, resolution - (RESSTEP * 2));
            analogWrite(ENABLE, resolution);
            break;
        }
        break;
  
      case PAUSE:
        switch(volButtonMapping){
          case CONTROLTEMP:
            setTemp = DEFAULTTEMP;
            break;
          case CONTROLHUMID:
            setHumid = DEFAULTHUMID;
            break;
          case CONTROLFAN:
            if(resolution == OFF){
              resolution = DEFAULTRES;
            } else {
              resolution = OFF;
            }
            analogWrite(ENABLE, resolution);
            break;
        }
        break;
        
      case EQ:
        // In auto mode, vol buttons can control temp threshold or humidity % threshold
        if(mode == AUTO) {
          volButtonMapping=(volButtonMapping+1)%2;
        }
        break;
      
      // allow for changing increment size for FASTFOR and REWIND
      // unfortunately no visual for this, so maybe leave out for final product
      case UP:
         switch(volButtonMapping){
            incrment = min(20, increment * 2);
         }
         break;
      case DOWN:
         switch(volButtonMapping){
            increment = max(5, increment / 2);
         }
         break;
      
      case POWER:
        PoweredOff()
        break;

      
      case STREPT:
        ManualNavigation();
        break;     
      
      //Maybe implement later?
      case HOLD: 
        break;
      
      default: 
        break;
  }
}

// these potentially don't work and could be cleaned up

// powered off state
void PoweredOff(){
  lcd.clear();
  
  // probably a problematic area
  bool deviceOff = true;
  irrecv.resume(); // receive the next value
  while(deviceOff == true){
    if (irrecv.decode(&results)){
      if(results == POWER){ // should re-enable device
        deviceOff = false;
      }
      else irrecv.resume(); // receive the next value
    }
    delay(100);
  }
  // end of potentially problematic area 
}

// user manual
void ManualNavigation(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("USER MANUAL");
  lcd.setCursor(0, 1);
  lcd.print("#:PAGE/ST:EXIT");
  
  // another potential problematic area
  bool looking = true;
  irrecv.resume(); // receive the next value
  while(looking == true){
    if (irrecv.decode(&results)){
      switch
        case ZERO:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("POWER TURNS");
          lcd.setCursor(0, 1);
          lcd.print("DEVICE ON/OFF");
          irrecv.resume(); // receive the next value
          break;
        case ONE:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("FUNC SWITCHES");
          lcd.setCursor(0, 1);
          lcd.print("MODE AUTO/MANU");
          irrecv.resume(); // receive the next value
          break;
        case TWO:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("IN MANUAL MODE");
          lcd.setCursor(0, 1);
          lcd.print("CTRL FAN SPEED");
          irrecv.resume(); // receive the next value
          break;
        case THREE:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("IN AUTO MODE");
          lcd.setCursor(0, 1);
          lcd.print("CTRL THREASHOLD");
          irrecv.resume(); // receive the next value
          break;
        case FOUR;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("THRESHOLDS");
          lcd.setCursor(0, 1);
          lcd.print("TEMP/HUMID");
          irrecv.resume(); // receive the next value
          break;
        case FIVE:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("EQ CHANGES");
          lcd.setCursor(0, 1);
          lcd.print("THRESHOLD");
          irrecv.resume(); // receive the next value
          break;
        case SIX:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PAUSE RESETS");
          lcd.setCursor(0, 1);
          lcd.print("TOGGLES FAN");
          irrecv.resume(); // receive the next value
          break;
        case SEVEN:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("VOL +/-");
          lcd.setCursor(0, 1);
          lcd.print("MODIFY BY 1");
          irrecv.resume(); // receive the next value
          break;
        case EIGHT:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("FORWARD/REWIND");
          lcd.setCursor(0, 1);
          lcd.print("MODIFY BY INC");
          irrecv.resume(); // receive the next value
          break;
        case NINE:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("UP/DOWN ARROWS");
          lcd.setCursor(0, 1);
          lcd.print("MODIFY INC SIZE");
          irrecv.resume(); // receive the next value
          break;
        case ST:
          looking = false;
          break;
        default:
          irrecv.resume(); // receive the next value
          break;
    }
    delay(100);
  }
  // end of potentially problematic area
}
