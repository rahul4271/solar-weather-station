#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

String apiKey = "  ";    // 2737PEGIY2F9413T Edit this CHANNEL API key according to your Account
String Host_Name = "  ";            // Edit Host_Name
String Password = "   ";          // Edit Password

SoftwareSerial ser(3, 2);              // RX, TX
DHT dht(5, DHT22);              // Initialising Pin and Type of DHT

int sensor_pin_soil = A0;
float moisture_percent_soil;
int sensor_analog;

const byte interruptPin = 13;
volatile boolean checkInterrupt = false;
int numberOfInterrupts = 0;
 
unsigned long debounceTime = 1000;
unsigned long lastDebounce=0;

void setup()
{ 
  Serial.begin(115200);                  // enable software serial
  ser.begin(115200);                     // reset ESP8266
  ser.println("AT+RST");               // Resetting ESP8266
  dht.begin();                        // Enabling DHT11
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  
lcd.init();//Initializing display
lcd.backlight();

  String cmd1 = "AT+CWMODE";
         cmd1+= "=";
         cmd1+= "3";
  ser.println(cmd1); 
  Serial.println(cmd1);

  char inv ='"';
  String cmd = "AT+CWJAP";
       cmd+= "=";
       cmd+= inv;
       cmd+= Host_Name;
       cmd+= inv;
       cmd+= ",";
       cmd+= inv;
       cmd+= Password;
       cmd+= inv;
ser.println(cmd);                    // Connecting ESP8266 to your WiFi Router
Serial.println(cmd);
}

void handleInterrupt() {
  checkInterrupt= true;
}
void loop()
{
  int humidity =  dht.readHumidity();             // Reading Humidity Value
  int temperature = dht.readTemperature();        // Reading Temperature Value

  sensor_analog = analogRead(sensor_pin_soil);
  moisture_percent_soil = ( 100 - ( (sensor_analog/1023.00) * 100 ) );
  Serial.print("Moisture Percentage of soil = ");
  Serial.print(moisture_percent_soil);
  Serial.print("%\n\n");
  delay(1000);

  if(checkInterrupt == true && ( (millis() - lastDebounce)  > debounceTime )){
 
      lastDebounce = millis();
      checkInterrupt = false;
      numberOfInterrupts++;
 
      Serial.print("Rain detected ");
      Serial.println(numberOfInterrupts); 
 
  }else checkInterrupt = false;

  String state1=String(humidity);                 // Converting them to string 
  String state2=String(temperature);              // as to send it through URL
  String state3=String(moisture_percent_soil);
  String state4=String(numberOfInterrupts);

  String cmd = "AT+CIPSTART=\"TCP\",\"";          // Establishing TCP connection
  cmd += "184.106.153.149";                       // api.thingspeak.com
  cmd += "\",80";                                 // port 80

  ser.println(cmd);
  Serial.println(cmd);

 if(ser.find("Error"))
 {
    Serial.println("AT+CIPSTART error");
    return;
 }
String getStr = "GET /update?api_key=";         // prepare GET string
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(state1);                       // Humidity Data
  getStr +="&field2=";
  getStr += String(state2);                       // Temperature Data
  getStr +="&field3=";
  getStr += String(state3);
  getStr +="&field4=";
  getStr += String(state4);
  getStr += "\r\n\r\n";

  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());                // Total Length of data

  ser.println(cmd);
  Serial.println(cmd);

  if(ser.find(">"))
  {
    ser.print(getStr);
    Serial.print(getStr);
  }
  else
  {
    ser.println("AT+CIPCLOSE");                  // closing connection
    Serial.println("AT+CIPCLOSE");
  }
 delay(1000);                             
lcd.clear();                 // for printing data on lcd
lcd.setCursor(0,0);
lcd.print("Temperature");
lcd.setCursor(0,1);           //Defining positon to write from second row,first column .
lcd.print(temperature);
delay(3000); 
lcd.clear();//Clean the screen
lcd.setCursor(0,0); 
lcd.print(" HUMIDITY ");
lcd.setCursor(0,1);
lcd.print(humidity);
delay(3000);

lcd.clear();//Clean the screen
lcd.setCursor(0,0); 
lcd.print(" moisture_percentage_soil ");
lcd.setCursor(0,1);
lcd.print(moisture_percent_soil);
delay(3000);
lcd.clear();//Clean the screen
lcd.setCursor(0,0); 
lcd.print(" RAINFALL ");
lcd.setCursor(0,1);
lcd.print(numberOfInterrupts);
delay(3000);
lcd.clear();
}
