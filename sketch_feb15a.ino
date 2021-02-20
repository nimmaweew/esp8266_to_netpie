#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#define DHTPIN D3 //D3 ค่าตั้งต้น
#define DHTTYPE DHT11 //ค่าตั้งต้น 
#define LED1 D4 //D4

unsigned long period = 300000; //ดีเลส่งข้อมูล

unsigned long last_time = 0; //ตั้งต้น 0

const char* ssid = "TOT-FAC1"; // Banpho = TOT-FAC1 / Bangplee = test-virus

const char* password = "1357qwer"; // Banpho = 1357qwer / Bangplee = 1q2w9o0p

const char* mqtt_server = "mqtt.netpie.io"; //ห้ามเปลี่ยน

const int mqtt_port = 1883; //ห้ามเปลี่ยน 

const char* mqtt_Client = "33b8338e-ff27-4558-a45a-d712fecf9da9"; 
//SVR01 = 6fb7c4de-8d7a-4144-8ae7-7f365e0296ca / DVR01 = 546777b0-9e28-41d2-acad-43ce56d3c3a0 /SVR02 = 33b8338e-ff27-4558-a45a-d712fecf9da9

const char* mqtt_username = "eWfY6YoBdhjqAjT8YE51ht9MC9x1MeFk"; 
//SVR01 = ABuKszGHP7Z7gLi29bPP77fjqvLyNTmi / DVR01 = n6byFu4jhuM96zpWhrLJzL7JRQQjDa4v / SVR02 = eWfY6YoBdhjqAjT8YE51ht9MC9x1MeFk

const char* mqtt_password = "!0cTLpzK$ejU5c3-QrZeQ65mT~vUtP5#"; 
//SVR01 = pruHiQ0Q!sBbG-2LF$QekTsbmEso#aKA / DVR01 = 51$QXn3EWNA_LvD!au3aZ)1LWvx2G1XA / SVR02 = !0cTLpzK$ejU5c3-QrZeQ65mT~vUtP5#

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
char msg[50];

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection…");
        if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
            Serial.println("connected");
            client.subscribe("@msg/led");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String message;
    for (int i = 0; i < length; i++) {
        message = message + (char)payload[i];
    }
    Serial.println(message);
    if(String(topic) == "@msg/led") {
        if(message == "on"){
            digitalWrite(LED1,0);
            client.publish("@shadow/data/update", "{\"data\" : {\"led\" : \"on\"}}");
            Serial.println("LED on");
        }
        else if (message == "off"){
            digitalWrite(LED1,1);
            client.publish("@shadow/data/update", "{\"data\" : {\"led\" : \"off\"}}");
            Serial.println("LED off");
        }
    }
}

void setup() {
    ESP.wdtDisable();//ปิดการทำงาน
    ESP.wdtEnable(WDTO_8S);//เปิดการทำงานรีเซ็ทระบบเมื่อทำงานในลูปเกิน 8 วินาที
    Serial.println("Arduino Reset");
    pinMode(LED1,OUTPUT);
    Serial.begin(115200);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    dht.begin();
}

void loop() {
    int humidity = dht.readHumidity();
    int temperature = dht.readTemperature();
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    if( millis() - last_time > period) {
    String data = "{\"data\": {\"humidity\":" + String(humidity) + ", \"temperature\":" + String(temperature) + "}}";
    Serial.println(data);
    data.toCharArray(msg, (data.length() + 1));
    client.publish("@shadow/data/update", msg);
    last_time = millis();
     
    }
   ESP.wdtFeed();//รีเซ้ทเวลาใหม่
    
}
