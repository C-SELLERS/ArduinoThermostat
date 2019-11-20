/*
CSS 427 UWB
CODE FOR ARDUINO THERMOSTAT
DHT SENSOR

*/


#include <dht_nonblocking.h>
#include <IRremote.h>
#include <LiquidCrystal.h>

//DHT sensor definition
#define DHT_SENSOR_TYPE DHT_TYPE_11

//Definition for remote functions
#define POWER   0xFFA25D
#define FUNC    0xFFE21D
#define VOLUP   0xFF629D
#define REWIND  0xFF22DD
#define PAUSE   0xFF02FD
#define FASTFOR 0xFFC23D
#define DOWN    0xFFE01F
#define VOLDOWN 0xFFA857
#define UP      0xFF906F
#define EQ      0xFF9867
#define STREPT 0xFFB04F
#define ZERO    0xFF6897
#define ONE     0xFF30CF
#define TWO     0xFF18E7
#define THREE   0xFF7A85
#define FOUR    0xFF10EF
#define FIVE    0xFF38C7
#define SIX     0xFF5AA5
#define SEVEN   0xFF42BD
#define EIGHT   0xFF4AB5
#define NINE    0xFF52AD
#define HOLD    0xFFFFFFFF

//Definition for volume button mapping
#define CONTROLTEMP   0
#define CONTROLHUMID  1

//Definition for operating modes
#define MANUAL  true
#define AUTO    false
 
//Initialize LCD Screen
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
//                RS  E  D4 D5  D6 D7

//Initialize DHT Sensor variables
static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

//Sensor Variables
float temperature;
float humidity;

//Variables for settings
int setTemp = 30;
int setHumid = 45;
bool manualOn = false; //Manual overide for on
bool on = false; //Current signal to fan
bool mode = MANUAL; //System's current mode
int volButtonMapping = CONTROLTEMP; //Indicate which threshold VOL buttons will change

//Relay Pin 13;
int relay = 13; 

/*
//ONLY FOR SIMULATION TMP VARIABLES
int sensorPin=0;
int reading;
float voltage;
*/

//Initialize the remote control variables
int receiverPin = 3;
IRrecv irrecv(receiverPin);
decode_results results;      

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  pinMode(relay,OUTPUT);
  irrecv.enableIRIn(); // Start the receiver
}

void loop(){
  
   
   
    //Measure temperature and humidity
  if( measure_environment( &temperature, &humidity )){
      Serial.print( "Temperature = " );
      Serial.print( temperature, 1 );
      Serial.print( " deg. C, Humidity = " );
      Serial.print( humidity, 1 );
      Serial.println( "%" );
    } else {
      Serial.println("no reading");
    }
     
  /*
  //ONLY FOR SIMULATION
  reading = analogRead(sensorPin);  
  voltage = (reading * 5.0) / 1024.0;
  temperature = (voltage - 0.5) * 100 ;  
  */
  
  	//Print to LCD 
    lcd.setCursor(0, 0);
    lcd.print(temperature);
    lcd.print(" C ");

    switch(on){
      case true:
        lcd.print("ON ");
        break;
      case false:
        lcd.print("OFF");
        break;
    }
    
    lcd.setCursor(0,1);
  	lcd.print("FAN ON AT ");
    lcd.print(setTemp);
    lcd.print(" C");
  
    lcd.setCursor(9,1);
    lcd.print(setHumid);
    lcd.print("%");
    
    
  
  //Relay condition to fan
    if(mode = AUTO) {
      if(temperature > setTemp || humidity > setHumid) turnOn();
      else turnOff();
    } else if(mode = MANUAL){
      if(manualOn) turnOn();
      else turnOff();
    }
  
  //Remote commands 
    //TODO: ISR? Seperate method!
    if (irrecv.decode(&results)){   
      switch(results.value){

        //Switch the mode that the device is in
        case POWER:
          if(mode == MANUAL) {
            //Manual off goes to manual on
            if(!manualOn) {
              manualOn = true;
            }
            //Manual on goes to auto
            else {
              mode = AUTO;
            }
          }
          //Auto goes to manual off
          else {
            mode = MANUAL;
            manualOn = false;
          }
          break;
        
        //Raise the threshold indicated by volButtonMapping
        case VOLUP:
          switch(volButtonMapping){
            case CONTROLTEMP:
              setTemp++;
              break;
            case CONTROLHUMID:
              setHumid++;
              break;
          }
          printThresholdChange();
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
          }
          printThresholdChange();
          break;
          
        case FUNC:
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
          delay(5000);
          break;

        case 
        
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
        
        case HOLD: 
          lcd.clear(); 
          lcd.setCursor(0,0); 
          lcd.print("REPEAT");
          break;
        
        default: 
          lcd.clear(); 
          lcd.setCursor(0,0); 
          lcd.print(results.value, HEX);
          break;
  }  
    delay(500);
    irrecv.resume(); // receive the next value
  }

  delay(100);
}




//Take measurement of environment, if fails to measure return false. 
static bool measure_environment( float *temperature, float *humidity ) {
    static unsigned long measurement_timestamp = millis( );
  
    if( millis( ) - measurement_timestamp > 1000ul ) {
      if( dht_sensor.measure( temperature, humidity )) {
          measurement_timestamp = millis( );
          return(true);
      }
    }
    return(false);
}

void printThresholdChange() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IN AUTO MODE, FAN ON AT ");
  lcd.setCursor(0,1);
  switch(volButtonMapping) {
    case CONTROLTEMP:
      lcd.print(setTemp);
      lcd.print("C");
      break;
    case CONTROLHUMID:
      lcd.print(setHumid);
      lcd.print("% HUMIDITY");
      break;
  }
  delay(2000);
}

void turnOn(){
  if(!on){
      digitalWrite(relay, HIGH);
      on = true;
    }
}

void turnOff(){
  if(on){
    digitalWrite(relay, LOW);
    on = false;
  }
}
