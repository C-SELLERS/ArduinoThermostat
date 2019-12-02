#include <dht_nonblocking.h>
#include <IRremote.h>
#include <LiquidCrystal.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11
 
//                BS  E  D4 D5  D6 D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
float temperature;
float humidity;
int receiver = 3; // Pin 12 on Arduino 
int relay = 13; // relay pin interface

//THIS IS FOR PROGRAMMING THE REMOTE CONTROLLER.
/*
  case 0xFFA25D: Serial.println("POWER"); break;
  case 0xFFE21D: Serial.println("FUNC/STOP"); break;
  case 0xFF629D: Serial.println("VOL+"); break;
  case 0xFF22DD: Serial.println("FAST BACK");    break;
  case 0xFF02FD: Serial.println("PAUSE");    break;
  case 0xFFC23D: Serial.println("FAST FORWARD");   break;
  case 0xFFE01F: Serial.println("DOWN");    break;
  case 0xFFA857: Serial.println("VOL-");    break;
  case 0xFF906F: Serial.println("UP");    break;
  case 0xFF9867: Serial.println("EQ");    break;
  case 0xFFB04F: Serial.println("ST/REPT");    break;
  case 0xFF6897: Serial.println("0");    break;
  case 0xFF30CF: Serial.println("1");    break;
  case 0xFF18E7: Serial.println("2");    break;
  case 0xFF7A85: Serial.println("3");    break;
  case 0xFF10EF: Serial.println("4");    break;
  case 0xFF38C7: Serial.println("5");    break;
  case 0xFF5AA5: Serial.println("6");    break;
  case 0xFF42BD: Serial.println("7");    break;
  case 0xFF4AB5: Serial.println("8");    break;
  case 0xFF52AD: Serial.println("9");    break;
  case 0xFFFFFFFF: Serial.println(" REPEAT");break;  
*/

//-----( Declare objects )----- - for remote 
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);
  pinMode(relay,OUTPUT);
  irrecv.enableIRIn(); // Start the receiver
}
/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
  if( millis( ) - measurement_timestamp > 1000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }
  return( false );
}


void loop()
{
  /* Measure temperature and humidity.  If the functions returns
     true, then a measurement is available. */
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "Temperature = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, Humidity = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
  }
// CODE TO CONTROL LCD MONITOR 
  lcd.setCursor(0, 0);
  lcd.print("Temp = ");
  lcd.print(temperature);
  lcd.print(" C");
  
// CODE TO CONTROL LOGIC OF FAN INITIATION
  if(temperature > 30.0 || humidity > 45)
  {
    digitalWrite(relay, HIGH); 
  }
  else if(temperature < 30.0 || humidity < 45)
  {
    digitalWrite(relay, LOW);
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
