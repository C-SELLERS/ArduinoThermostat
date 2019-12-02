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
int ReadTime = 1000; 
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
bool changes = true; //true for initial bootup
bool manualOn = false; //Manual overide for on
bool on = false; //Current signal to fan
bool mode = MANUAL; //System's current mode
int volButtonMapping = CONTROLTEMP; //Indicate which threshold VOL buttons will change

//Relay Pin 13;
int relay = 13; 


//Initialize the remote control variables
int receiverPin = 3;
IRrecv irrecv(receiverPin);
decode_results results;    


void setup(){
    lcd.begin(16, 2);
    Serial.begin(9600);
    pinMode(relay,OUTPUT);
    irrecv.enableIRIn(); // Start the receiver
}


void loop(){
    //Read the temp and humidity
    Time = millis();
    if(Time - Checkpoint > ReadTime){
        Checkpoint = Time;
        measureConditions();
    }

    if(changes){
        if(curTemp > setTemp || curHum > setHumid || manualOn) {
            if(!on){
                digitalWrite(relay, HIGH); 
                on = true;
            }
        } else {
            if(on){
                digitalWrite(relay, LOW);
                on = false;
            }
        }

        updateScreen();
        changes = false;

    }

  
    
    
    /*
    if (irrecv.decode(&results)) // have we received an IR signal?
    {   
        switch(results.value)
        {
        case 0xFF30CF: 
        Serial.println(tempC);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp         C  ");
        lcd.setCursor(6,0);
        lcd.print(tempC);
        delay(5000);
        break;
        
        case 0xFF18E7:
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("Give Us"); 
        delay(2000); 
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("A Pass");
        delay(1000);
        break;    
        
        case 0xFF7A85:
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("Matt is"); 
        delay(2000); 
        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("Sexy");
        delay(1000);
        break;    
        
        case 0xFFFFFFFF: Serial.println(" REPEAT");break;  
        default: 
        Serial.println(" other button : ");
        Serial.println(results.value);
        }  
    delay(500);
        irrecv.resume(); // receive the next value
    }*/
    }

//Void for changing screen if changes occur
void updateScreen(){
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
        changes = true;
    }
    
}