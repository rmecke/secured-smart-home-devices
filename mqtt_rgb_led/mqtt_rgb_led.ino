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
const char* client_name = "RGB-LED";
const char* ssid = "WLAN-AKX7YW";
const char* password = "7527024998732131";
const char* mqtt_server = "192.168.2.120";
WiFiClient espClient;
PubSubClient client(espClient);

// Sign Of Life
#include <time.h>
int lastSign = 0;

// Color
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int RED_LED= 16;
int GREEN_LED = 5;
int BLUE_LED = 4;
boolean valueSwitch = false;
int valueIntensity = 0;
String valueColor = "red";

// Topics
const char* topic_switch = "lab/rgb-led/switch";
const char* topic_intensity = "lab/rgb-led/intensity";
const char* topic_color = "lab/rgb-led/color";
const char* topic_heartbeat = "lab/rgb-led/heartbeat";
const char* topic_restart = "lab/rgb-led/restart";

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
      client.subscribe(topic_switch);
      client.subscribe(topic_intensity);
      client.subscribe(topic_color);
      client.subscribe(topic_restart);
    } else {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);
 
        WiFi.begin(ssid, password);
 
        while (WiFi.status() != WL_CONNECTED) {
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
        delay(10);
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

  if (strcmp( topic, topic_switch)==0) {
    String str = "";
    for (int i = 0; i < length; i++) { str += (char)payload[i]; }

    if (str == "true") {
      valueSwitch = true;
    } else {
      valueSwitch = false;
    }

    handleLed();
  }

  if (strcmp( topic, topic_intensity)==0) {
    String str = "";
    for (int i = 0; i < length; i++) { str += (char)payload[i]; }

    valueIntensity = str.toInt();

    handleLed();
  }

  if (strcmp( topic, topic_color)==0) {
    String str = "";
    for (int i = 0; i < length; i++) { str += (char)payload[i]; }

    valueColor = str;

    handleLed();
  }

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

void handleLed() {  
  if (valueSwitch == true) {
    digitalWrite(BUILTIN_LED, LOW); // LOW for ON, because it is active low on the ESP-01)
    float valueRed = 0;
    float valueGreen = 0;
    float valueBlue = 0;

    if (valueColor == "red") {
      valueRed = 255;
    }

    if (valueColor == "green") {
      valueGreen = 255;
    }

    if (valueColor == "blue") {
      valueBlue = 255;
    }

    float intensity = valueIntensity;

    valueRed = (valueRed/255) * (intensity/100*255);
    valueGreen = (valueGreen/255) * (intensity/100*255);
    valueBlue = (valueBlue/255) * (intensity/100*255);

    analogWrite(RED_LED, valueRed);
    analogWrite(GREEN_LED, valueGreen);
    analogWrite(BLUE_LED, valueBlue);
    digitalWrite(BUILTIN_LED, LOW);
  } else {
    analogWrite(RED_LED, 0);
    analogWrite(GREEN_LED, 0);
    analogWrite(BLUE_LED, 0);
    digitalWrite(BUILTIN_LED, HIGH);
  }
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

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    client.unsubscribe(topic_switch);
    client.unsubscribe(topic_intensity);
    client.unsubscribe(topic_color);
    client.unsubscribe(topic_restart);

    reconnect();
  }
  client.loop();

  signOfLife();
}
