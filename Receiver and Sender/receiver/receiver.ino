#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <WiFi101.h>
#include "arduino_secrets.h"
#include <Wire.h>
#include "rgb_lcd.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID
char pass[] = SECRET_PASS;    // your network password

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "/projetalarme/alarme";
const char topic2[]  = "/projetalarme/active";
int receptionArd1;
int active;

//set interval for sending messages (milliseconds)
const long interval = 1000;
unsigned long previousMillis = 0;


//init valeur lecteur carte
char d;
char valeur_carte[12];
char valeur_cle[12]={'6','0','0','0','3','9','6','0','D','D','E','4'};
int compteur=0;
int tmp=1;
int validCarte = 0;

int setAlarme;

//init val LCD 
rgb_lcd lcd;

int colorR = 255;
int colorG = 255;
int colorB = 255;


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  Serial1.begin(9600);
  //init lcd
  lcd.begin(16, 2);


  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);

  // subscribe to a topic
  mqttClient.subscribe(topic);
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.println();
}

void loop() {
  // call poll() regularly to allow the library to receive MQTT messages and
  // send MQTT keep alive which avoids being disconnected by the broker
  mqttClient.poll();

  //Traitement capteur de carte
  if(Serial1.available()){
  d=Serial1.read();
  if(d!=0x02&&d!=0x03){//ce sont des valeurs qui entourent l'id de la carte , elles ne permettent pas d'avoir l'id de la carte 
    valeur_carte[compteur]=d;
    compteur++;
  }else if(d==0x03){//le d ici correspond a la derniere valeur qui entoure le code de la carte 
   compteur=0;
   tmp=0;
 
   if(strcmp(valeur_carte, valeur_cle) == 0 && tmp == 0){
    validCarte = 1;
    Serial.println("carte validée");
   }
  }
}

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

  Serial.print("Sending message to topic: ");
  Serial.println(topic2);

  /*ENVOIE JSON */
  
  StaticJsonDocument<200> doc;
  doc["alarme2"] = "activation de l'alarme2";
  JsonArray data_arduino2 = doc.createNestedArray("data_arduino2");
  data_arduino2.add(setAlarme);

  String message2;
  serializeJson(doc, message2);
    
  mqttClient.beginMessage(topic2);
  mqttClient.print(message2);
  mqttClient.endMessage();

  Serial.println("message :" );
  Serial.println(message2);
  Serial.println("fin message");
  Serial.println();

  setAlarme = 0;
  validCarte = 0;
  
  }


}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  String messageR = "";

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    messageR += (char)mqttClient.read();    
  }

  Serial.println("Message recu : ");
  Serial.println(messageR);

  //RECEPTION JSON
  StaticJsonDocument<200> doc;
  deserializeJson(doc, messageR);

  JsonObject obj = doc.as<JsonObject>();

  receptionArd1 = obj["data"][0];
  Serial.print("receptionArd1: ");
  Serial.println(receptionArd1);
  if(receptionArd1 ==1){
    active = 1;
  }
  Serial.print("validCarte: ");
  Serial.println(validCarte);
  alarme(active, validCarte);

}


void alarme(int var, int lectCarte) {
  if(var == 0) { // l'alarme est désactivée
    colorG = 255;
    colorR = 0;
    colorB = 0;
    lcd.clear();
    lcd.setRGB(colorR, colorG, colorB);
    lcd.setCursor(0,0);
    lcd.print("alarme desactivee");
    setAlarme = 0;
  }else if(var == 1) { // on active l'alarme via setAlarme = 2;
    colorG = 0;
    colorR = 255;
    colorB = 0;
    lcd.clear();
    lcd.setRGB(colorR, colorG, colorB);
    lcd.setCursor(0,0);
    lcd.print("alarme activee");
    setAlarme = 2;
    }
  if(lectCarte == 1){ // on passe la carte et on desactive l'alarme via setAlarme = 3;
    setAlarme = 3;
    lectCarte = 0;
    active = 0;
  }
}
