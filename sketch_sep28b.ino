/*******************************************************************************
 * Luceiia - Automated Thermostat
 * 
 * This application, mounted on an X-Board_Relay with an attached SHT1X Humidity 
 * and Temperature Sensor, controls a local fernace. The furnace is activated by 
 * triggering one of the relays on the board. 
 * 
 * On boot up, the application connects to the network, obtains an IP and 
 * prepares to respond to incoming http requests. External services can then 
 * connect to the device and set the desired temperature. This application will 
 * monitor the attached humidity and temperature sensors and to determine when 
 * the furnace should be activated.
 *
 * Date:   October 11, 2013
 * Author: Scott Steil
 * 
 */
 
#include <SPI.h>
#include <Ethernet.h>
#include <SHT1x.h>
#include <TextFinder.h>

boolean debug = true;
float maxTemperature = 25.0;
float minTemperature = 15.0;
float curTemperature = 23.0;  

float temperatureRangeHi = 1.0;    // the margin of temperature difference we allow before shutting off the furnace
float temperatureRangeLo = 1.0;    // the margin of temperature difference we allow before turning on the furnace
boolean isHeating = false;


// Pins
#define Sensor1ClockPin 2
#define Sensor1DataPin  3
#define FurnaceRelay    8


// Ethernet
EthernetServer server(80);
byte mac[] = { 0xDE, 0xCD, 0xAE, 0x0F, 0xFE, 0xED };


// SHT1x Humidity and Temperature Sensor
SHT1x sensor1(Sensor1DataPin, Sensor1ClockPin);



void setup() {
  
  Serial.begin(9600);
  while (!Serial) {
  ; // Open serial communications and wait for port to open. Needed for Leonardo only
  }
  
  // start the Ethernet connection:
  while (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP. Reattempting in 10 seconds.");
    delay(10000);
  }
  
  // start the Ethernet connection and the server:
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  
  deactivateFurnace();
}

 
 
void loop() {
  manageFurnace();
  manageConnections();
}

void manageConnections() {
  EthernetClient client = server.available();
  
  if (client) {
    TextFinder finder( client );
    
    while(client.connected()) {
      if (client.available()) {
        
//        char c = client.read();
//        Serial.write(c);
//        if (c == '\n') break;
    
        if( finder.find("GET /") ) {
          float temp = finder.getFloat();
          
          if (inRange(temp))
          {
            Serial.print("Setting temperature to ");
            Serial.println(temp);
            curTemperature = temp;
          }
          
          client.flush();
        
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println();
          
          printSensor(client, false);
          client.stop();
        }
        
        break;
      }
    }
    
    delay(1); // give the web browser time to receive the data
    client.stop();
    Serial.println("Client disconnected");
  }
}

boolean inRange(float temperature) {
  return temperature < maxTemperature && temperature > minTemperature;
}

void printSensor(EthernetClient client, boolean detailed) {
  // It takes a long time to calculate the heat index, so I've disabled it for now
  float temp_hic;
  float temp_hif;
  float temp_c   = averageTemperatureC();
  float temp_f   = averageTemperatureF();
  float humidity = averageHumidity();
  
  if (detailed) {
    temp_hic = heatIndexTemperatureC();
    temp_hif = heatIndexTemperatureF();
  }

  client.print("{");
    client.print("\"temperature\": {");
      client.print("\"celcius\": ");
      client.print(temp_c, DEC);
      client.print(", \"fahrenheit\": ");
      client.print(temp_f, DEC);
    client.print("}");
    
    if (detailed) {
      client.print(", \"heatIndexTemperature\": {");
        client.print("\"celcius\": ");
        client.print(temp_hic, DEC);
        client.print(", \"fahrenheit\": ");
        client.print(temp_hif, DEC);
      client.print("}");
    }
    
    client.print(", \"humidity\": ");
    client.print(humidity);
    client.print(", \"currentSetting\": ");
    client.print(curTemperature);
    client.print(", \"isHeating\": ");
    client.print(isHeating ? "true" : "false");
  client.print("}");
}




void manageFurnace() {
  //float temperature = heatIndexTemperature();
  float temperature = averageTemperatureC();
  
  if (isHeating && temperature > curTemperature + temperatureRangeHi) {
      deactivateFurnace();
  } 
  else if (!isHeating && temperature < curTemperature - temperatureRangeLo) {
      activateFurnace();
  }
}

void activateFurnace() {
  int pin = 7;
  digitalWrite(pin, HIGH);
  isHeating = true;
}

void deactivateFurnace() {
  int pin = 7;
  digitalWrite(pin, LOW);
  isHeating = false;
}

float heatIndexTemperatureC() {
  return f2c(heatIndexTemperatureF());
}

float heatIndexTemperatureF() {
  float fahrenheit = averageTemperatureF();
  float humidity   = averageHumidity();
  
  if (humidity > 100) humidity = 100;
  if (humidity < 0  ) humidity = 0;
  
  return ersatzHeatIndex(fahrenheit, humidity);
}

float averageTemperatureC() {
  // when we have two sensors we will average them
  return sensor1.readTemperatureC();
}

float averageTemperatureF() {
  // when we have two sensors we will average them
  return sensor1.readTemperatureF();
}

float averageHumidity() {
  // when we have two sensors we will average them
  return sensor1.readHumidity();
}

float ersatzHeatIndex(float fahrenheit, float humidity) {
  float t = fahrenheit;
  float h = humidity;
  
  // ref: https://en.wikipedia.org/wiki/Heat_index
  return -42.379 + 2.04901523*t + 10.14333127*h - 0.22475541*t*h - 0.00683783*t*t - .05481717*h*h + .00122874*t*t*h + .00085282*t*h*h - .00000199*t*t*h*h;
}

float f2c (float fahrenheit) {
  return (fahrenheit - 32) / 1.8;
}

float c2f (float celcius) {
  return celcius * 1.8 + 32;
}
