#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
const char* SSID = "";
const char* PSK = "";
const char* MQTT_BROKER = "192.168.0.87";
 
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int powerPin = 5; // D1 power pin
int smallSizePin = 4; // D2 // Pin for small cup
int bigSizePin = 0;// D3 // Pin for big cup
int echoPin = 13; //D7 // Echo Pin
int trigPin = 12; //D6 // Trigger Pin
 
long duration, distance; // Duration used to calculate distance

void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();


 
void setup() {
    pinMode(powerPin, OUTPUT);
    pinMode(smallSizePin, OUTPUT);
    pinMode(bigSizePin, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    Serial.begin(115200);
    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);
    client.setCallback(callback);
}
 
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
 
    WiFi.begin(SSID, PSK);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

int ultra() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration/58.2;
  return distance;
}
 
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Received message [");
    Serial.print(topic);
    Serial.print("] ");
    char msg[length+1];
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        msg[i] = (char)payload[i];
    }
    Serial.println();
 
    msg[length] = '\0';
    Serial.println(msg);

    distance = ultra();
    Serial.print("Current distance: ");
    Serial.println(distance);

    if(strcmp(topic, "/kaffeemaschine/msg")==0){
    if(strcmp(msg,"power")==0){
        digitalWrite(5, HIGH);
        delay(1000);
        digitalWrite(5, LOW);
    }
    else if(strcmp(msg,"smallSize")==0 && distance<5){
        digitalWrite(smallSizePin, HIGH);
        delay(1000);
        digitalWrite(smallSizePin, LOW);
    }
    else if(strcmp(msg,"bigSize")==0 && distance<5){
        digitalWrite(bigSizePin, HIGH);
        delay(1000);
        digitalWrite(bigSizePin, LOW);
    }
    } 
}
 
void reconnect() {
    while (!client.connected()) {
        Serial.println("Reconnecting MQTT...");
        if (!client.connect("Kaffemaschine")) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
    client.subscribe("/kaffeemaschine/msg");
    Serial.println("MQTT Connected...");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
