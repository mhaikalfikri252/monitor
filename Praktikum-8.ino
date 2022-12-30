#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <stdlib.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Connect to Wifi
const char* ssid = "TP-Link_60DA";
const char* password = "h1ced3948b5";
const char* mqtt_server = "riset.revolusi-it.com";

const char* topik = "iot/G_241_21_0007";

String messages_terima, messages_kirim;

int sensor = A0;
float nilai_analog, nilai_volt, nilai_suhu;

void konek_wifi()
{
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
    if (client.connect("G.241.21.0007")) {
      Serial.println("connected");
      // ... and subscribe to topic
      client.subscribe(topik);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik...");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float baca_suhu() {
  nilai_analog = analogRead(sensor);
  nilai_volt = (nilai_analog/1024)*2.8;
  nilai_suhu = nilai_volt*100;

  Serial.println("Analog = " + (String)nilai_analog);
  Serial.println("Voltase = " + (String)nilai_volt);
  Serial.println("Suhu = " + (String)nilai_suhu + " Celcius");

  return nilai_suhu;

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan dari MQTT [");
  Serial.print(topic);
  Serial.print("] ");
  messages_terima="";

  for (int i=0;i<length;i++) { // susun jadi string saja...
    char receivedChar = (char)payload[i];
    messages_terima = messages_terima + (char)payload[i]; // ambil pesannya masukkan dalam string
    }
    Serial.println(messages_terima);

  float suhu = baca_suhu();
  // kirim pesan ke mqtt server
  messages_kirim=(String)suhu;

  if (suhu < 29) {
    digitalWrite(D1, 0);
    digitalWrite(D2, 0);
    digitalWrite(D3, 0);
  } else if (suhu >= 29 && suhu < 30) {
    digitalWrite(D1, 1);
    digitalWrite(D2, 0);
    digitalWrite(D3, 0);
  } else if (suhu >= 30 && suhu < 31) {
    digitalWrite(D1, 1);
    digitalWrite(D2, 1);
    digitalWrite(D3, 0);
  } else if (suhu >= 31) {
    digitalWrite(D1, 1);
    digitalWrite(D2, 1);
    digitalWrite(D3, 1);
  }

  // perhatikan cara ngirim variabel string lewat client.publish ini gak bisa langsung...
  char attributes[100];
  messages_kirim.toCharArray(attributes, 100);
  client.publish(topik,attributes,true);
  Serial.println("messages : "+messages_kirim+" terkirim...");
  delay(3000);

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(sensor, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
 if(WiFi.status() != WL_CONNECTED) { konek_wifi(); } // jika tidak konek wifi maka di reconnect
  if (!client.connected()) { reconnect(); } // reconnect apabila belum konek

  client.loop();

}
