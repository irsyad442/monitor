#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
// #include <Adafruit_INA219.h>
#include <stdlib.h>

#include "DHT.h"
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


WiFiClient espClient;
PubSubClient client(espClient);

// Connect to the WiFi
const char* ssid = "IRSYAD";
const char* password = "bismillah";
const char* mqtt_server = "riset.revolusi-it.com";

const char* topik = "iot/G_231_19_0147";

String messages_terima;
String messages_kirim;

int led1=D1;
int led2=D2;
int led3=D3;

int sensor=A0;
float nilai_analog, nilai_volt, nilai_suhu; // deklarasikan variabelnya bertipe float

void konek_wifi() {
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
  Serial.print("Menghubungkan diri ke MQTT Server : "+(String)mqtt_server);
  // Attempt to connect
  if (client.connect("G.231.19.0147-57691")) {
    Serial.println(" connected");
    // ... and subscribe to topic
    client.subscribe(topik);
  } else {
    Serial.print(" failed, rc=");
    Serial.print(client.state());
    Serial.println(" coba lagi dalam 5 detik...");
    // Wait 5 seconds before retrying
    delay(5000);
    }
  }
}

float baca_suhu_dht() {
  float kelembaban = dht.readHumidity();
  float suhu = dht.readTemperature();
  return suhu;
}

float baca_suhu() {
  nilai_analog = analogRead(sensor);
  // karena range voltase nya adalah 0 s/d 3.3v, dan nilai analog 1..1024 maka voltase sbb :
  nilai_volt = (nilai_analog/1024)*3.3;
  // karena nilai suhunya adalah 0..100 dan nilai analognya 0..1024 maka suhunya sbb :
  nilai_suhu = nilai_volt*100;

  Serial.println("Analog  = "+(String)nilai_analog);
  Serial.println("Voltase = "+(String)nilai_volt);
  Serial.println("Suhu    = "+(String)nilai_suhu+" Celcius");

  return nilai_suhu;
   
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan dari MQTT [");
  Serial.print(topic);
  Serial.print("] ");
  messages_terima="";
  for (int i=0;i<length;i++) { // susun jadi string saja...
    char receivedChar = (char)payload[i];
    messages_terima=messages_terima+(char)payload[i]; // ambil pesannya masukkan dalam string
  }
  
  Serial.println(messages_terima);

  // kirim pesan ke mqtt server
  float suhu = baca_suhu_dht();

  // messages_kirim = "{\"suhu":"+(String)suhu+"}";
  messages_kirim = (String)suhu;
  // Serial.println(messages_kirim);
  // perhatikan cara ngirim variabel string lewat client.publish ini gak bisa langsung...
  // eksekusi LED
  eksekusiLed(suhu);
  char attributes[100];
  messages_kirim.toCharArray(attributes, 100);
  client.publish(topik,attributes,true);
  Serial.println("messages : "+messages_kirim+" terkirim...");
  delay(1000);
}

void eksekusiLed(float suhu) {
  if(suhu<29) { // led mati semua
    digitalWrite(led1,0);
    digitalWrite(led2,0);
    digitalWrite(led3,0);
  }

  if(suhu>=29 && suhu<30) { // led nyala 1
    digitalWrite(led1,1);
    digitalWrite(led2,0);
    digitalWrite(led3,0);
  }

  if(suhu>=30 && suhu<31) { // led nyala 2
    digitalWrite(led1,1);
    digitalWrite(led2,1);
    digitalWrite(led3,0);
  }

  if(suhu>31) { // led nyala 3
    digitalWrite(led1,1);
    digitalWrite(led2,1);
    digitalWrite(led3,1);
  }
}

// -----------------------begin----------------------- //
void setup() {
  Serial.begin(9600);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(sensor, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  digitalWrite(led1,0);
  digitalWrite(led2,0);
  digitalWrite(led3,0);
  dht.begin();
}

void loop() {
  if(WiFi.status() != WL_CONNECTED) { konek_wifi(); } // jika tidak konek wifi maka di reconnect
  if (!client.connected()) { reconnect(); } //reconnect apabila belum konek

  client.loop();

}
