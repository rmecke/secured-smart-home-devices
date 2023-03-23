#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "WLAN-AKX7YW";
const char* password = "7527024998732131";
const char* mqtt_server = "192.168.2.120";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int LED     = LED_BUILTIN;
int MIK     = 16;
int POTI    = 0;
int valAnalog = 0;
int valDigital;
int clapped = 0;
int clapCounter = 0;
int lastClap = 0;
boolean ledStatus = false;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int clapCheck() {  
  //int valAnalog = analogRead(POTI);
  int valDigital = digitalRead(MIK);
  int result = 0;

  if (millis() - lastClap >= 200) {
    if (clapped == 0) {
      if (valDigital == 1) {
          clapped = 1;
          clapCounter += 1;
          lastClap = millis();
          Serial.println("Clapped");  
      }
    } else {
      if (valDigital == 0) {
        clapped = 0;
      }
    }
  }

  if (millis() - lastClap >= 1000 && clapCounter > 0) {
      Serial.println(clapCounter);
      result = clapCounter;
      client.publish("test/clap",String(clapCounter).c_str());
      clapCounter = 0;
  }

  return result;
}

void setup() {
  pinMode(BUILTIN_LED,OUTPUT);
  pinMode(MIK,INPUT);
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
  }

  /*
  //Serial.println(digitalRead(MIK));

  valAnalog = analogRead(POTI);
  //Serial.println(valAnalog, DEC);

  int data = digitalRead(MIK);
  if(data==1) {
    if (ledStatus==false) {
      ledStatus=true;
      digitalWrite(BUILTIN_LED,LOW);
    } else {
      ledStatus=false;
      digitalWrite(BUILTIN_LED,HIGH);
    }
  }*/

  clapCheck();
}
