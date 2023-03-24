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

int clapCheck(double sig) {
  
  if (millis() - lastClap < clapWindowMin) {
  
  } else if (millis() - lastClap < clapWindowMax) {
    if (clapCounter >= 1 && sig >= treshold) {
      clapCounter += 1;
      Serial.println(sig);
      lastClap = millis();
    }
  } else {
    if (clapCounter > 0) {
      Serial.print("clapped:");
      Serial.println(clapCounter);
      client.publish("test/clap",String(clapCounter).c_str());
      
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

  readAudio();
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
