#include <ESP8266WiFi.h>
#include <DHT.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"      
#define WLAN_SSID       "i net fiber"
#define WLAN_PASS       "903042394"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883 
#define AIO_USERNAME  "rahul4271"
#define AIO_KEY       "aio_AZEQ81MHwCG0YGsdhMbV24UMYnT1"
#define S0 D3
#define S1 D2
#define S2 D1

#define ANALOG_INPUT A0

float input_voltage = 0.0;  //for battery voltage
float temp=0.0;
float r1=75.0;
float r2=225.0;
float value;

WiFiClient client;

int humidity,temperature;
DHT dht(D4, DHT22);

float moisture_percent_soil;
int sensor_analog;
int sensor_rain;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish Battery = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/battery");
Adafruit_MQTT_Publish moisture = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soil");
Adafruit_MQTT_Publish rain = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/rain");



void setup() {
  Serial.begin(115200);
  Serial.println(F("Adafruit IO Example"));// Connect to WiFi access point.
  Serial.println(); 
  Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  connect();
  dht.begin(); 
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT); 
}
void changepin(int c, int b, int a) {
  digitalWrite(S0, a);
  digitalWrite(S1, b);
  digitalWrite(S2, c);
}

void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}


void loop()
{

   humidity =  dht.readHumidity(); 
   temperature = dht.readTemperature();

   changepin(LOW, LOW, HIGH);  //battery pin  001
   value = analogRead(ANALOG_INPUT); //Value of the sensor connected to channel 1 pin of Mux
   temp = (value * 5.0) / 1024.0; 
   input_voltage = temp / (r2/(r1+r2));
   if (input_voltage < 1) 
   {
     input_voltage=0.0;
   }
    if (input_voltage > 4.2) 
   {
     input_voltage=4.2;
   }
   delay(10);

  
  changepin(LOW, HIGH, LOW);  //soil sensor  010
  sensor_analog = analogRead(ANALOG_INPUT); //Value of the sensor connected to channel 2 pin of Mux
  moisture_percent_soil = ( 100 - ( (sensor_analog/1023.00) * 100 ) );
  delay(10);


  changepin(LOW, HIGH, HIGH);  ///rain sensor   011
  sensor_rain= analogRead(ANALOG_INPUT); //Value of the sensor connected to channel 3 pin of Mux
  sensor_rain = constrain(sensor_rain, 150, 440); 
  sensor_rain = map(sensor_rain, 150, 440, 1023, 0); 
  delay(500);

     if (! Temperature.publish(temperature)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    } 
     if (! Humidity.publish(humidity)) {                    
      Serial.println(F("Failed"));
    }
    if (! Battery.publish(input_voltage)) {                     
      Serial.println(F("Failed"));
    }
    if (! moisture.publish(moisture_percent_soil)) {                     
      Serial.println(F("Failed"));
    }
    if (! rain.publish(sensor_rain)) {       
      Serial.println(F("Failed"));
    }
    else {
      Serial.println(F("Sent!"));
    }
    delay(2000);
}
