#include <MQTT.h>
#include <MQTTClient.h>

/*
 ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

/*
 * TLS Support: https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/examples/WiFiClientSecure/WiFiClientSecure.ino
 */

//WLAN and MQTT
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const char* client_name = "ROOM-DATA";
const char* ssid = "WLAN-AKX7YW";
const char* password = "7527024998732131";
const char* mqtt_server = "192.168.2.120";
WiFiClient espClient;
PubSubClient client(espClient);

// Sign Of Life
#include <time.h>
int lastSign = 0;

// Clap
int LED     = LED_BUILTIN;
int MIK     = 16;
int POTI    = 0;
int valAnalog = 0;
int valDigital;
int clapped = 0;
int clapCounter = 0;
int lastClap = 5000;
boolean ledStatus = false;

const int sampleWindow = 20;
const int sampleWindowInner = 1;
unsigned int sample;

unsigned long startMillis = 10;
unsigned long startMillisInner = 0;
unsigned int signalMax = 0;
unsigned int signalMin = 1024;

const int clapWindowMin = 100;
const int clapWindowMax = 1000;

double treshold = 2;

// Temperature
#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT11
int TEMP_PIN = 5;
float temperature = 0;
float humidity = 0;
DHT dht(DHTPIN, DHTTYPE);
int lastRead = 0;

// Topics
const char* topic_clap = "my-room/room-data/clap";
const char* topic_temperature = "my-room/room-data/temperature";
const char* topic_humidty = "my-room/room-data/humidty";
const char* topic_heartbeat = "my-room/room-data/heartbeat";
const char* topic_restart = "my-room/room-data/restart";

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    clientId += client_name;
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_restart);
    } else {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);
 
        WiFi.begin(ssid, password);
 
        while (WiFi.status() != WL_CONNECTED) {
          lostProtocol();
          delay(500);
          Serial.print(".");
        }
      }
     
      
      int lastMillis = millis();
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      // Wait 5 seconds before retrying

      //MQTT Offline --> Lost-Protocol
      while (millis()-lastMillis < 5000) { 
        lostProtocol();
        delay(50);
      }      
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ( strcmp( topic, topic_restart ) == 0 )
  {  
    String restartStr = "";
    for (int i = 0; i < length; i++) { restartStr += (char)payload[i]; }

    if (restartStr == "true") {
      client.publish(topic_restart,String("false").c_str());
      Serial.println("Restart requested via MQTT."); 
      Serial.println("Restarting...");

      delay (1000);
      ESP.restart();
    }
  }
}

int clapCheck(double sig) {
  if (millis() - lastClap < clapWindowMin) {
  
  } else if (millis() - lastClap < clapWindowMax) {
    if (clapCounter >= 1 && sig >= treshold) {
      clapCounter += 1;
      Serial.println(sig);
      lastClap = millis();
    }
  } else {
    if (clapCounter > 1) {
      Serial.print("clapped:");
      Serial.println(clapCounter);
      client.publish(topic_clap,String(clapCounter).c_str());
      
      clapCounter = 0;
    } else {
      if (sig >= treshold) {
        clapCounter = 1;
        Serial.println(sig);
        lastClap = millis();
      }
    }
  }

  return 0;
}

int readAudio() {
  if (millis() - startMillis < sampleWindow) {
    if (millis() - startMillisInner > sampleWindowInner) {
      sample = system_adc_read();
      if (sample < 1024) {
          if (sample > signalMax)
          {
            signalMax = sample;
          }
          else if (sample < signalMin)
          {
            signalMin = sample;
          }
      }
      startMillisInner = millis();
    }
  }

  if (millis() - startMillis > sampleWindow) {
    startMillis = millis();
    unsigned int peakToPeak = signalMax - signalMin;
    double sig = (peakToPeak * 5.0) / 1024;
    signalMax = 0;
    signalMin = 1024;
    //Serial.print("Analog signal: ");
    //Serial.println(sig);

    clapCheck(sig);
  }

  return 0;
}

int readTemperature() {
  if (millis() - lastRead >= 1000) {
      temperature = dht.readTemperature();
      humidity = dht.readHumidity();
      String str = "Temperatur: " + String(temperature) + " °C // Luftfeuchtigkeit: " + String(humidity) + " %";
      // Serial.println(str);
      client.publish(topic_temperature,String(temperature).c_str());
      client.publish(topic_humidty,String(humidity).c_str());
      lastRead = millis();
  }
  

  return 0;
}


void lostProtocol() {

}

void signOfLife() {
  if (millis()-lastSign >= 60000) {
    lastSign = millis();
    client.publish(topic_heartbeat,String("true").c_str());
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
  pinMode(BUILTIN_LED,OUTPUT);
  pinMode(MIK,INPUT);
  pinMode(TEMP_PIN,INPUT);

  dht.begin();
}

void loop() {
  if (!client.connected()) {
    client.unsubscribe(topic_restart);
    
    reconnect();
  }
  client.loop();
  
  readAudio();
  readTemperature();
  signOfLife();
}
