#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <WiFi101.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "/projetalarme/alarme";
const char topic2[]  = "/projetalarme/active";

//set interval for sending messages (milliseconds)
const long interval = 1000;
unsigned long previousMillis = 0;


//INITIALISATION VARIABLES

#define SOUND_SENSOR A0
#define LED 3 // the number of the LED pin
#define THRESHOLD_VALUE 400//The threshold to turn the led on 400.00*5/1024 = 1.95v
int reception_ard2;


void setup() {

  // INITIALISATION VARIABLES 
    
  // Capteur de mouvement
  #define PIR_MOTION_SENSOR 2//Use pin 2 to receive the signal from the module
  pinMode(PIR_MOTION_SENSOR, INPUT);

  // Sound sensor
  pinMode(SOUND_SENSOR, INPUT);

  //LED
  pinMode(LED, OUTPUT);
  
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
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


  /// inscription topic retour
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(topic2);
  Serial.print("Topic2 retour: ");
  Serial.println(topic2);
  Serial.println();
}

void loop() {
 
  mqttClient.poll();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;


  //TRAITEMENT ENVOIE DONNEES

  int activeAlarme = 0;
  int sound = 404; 
  int mouv = 666;
  
  //Capteur de mouvement
  if(digitalRead(PIR_MOTION_SENSOR))//if it detects the moving people?
        mouv= 1; //"Hi,people is coming"
    else
        mouv=0; //"Watching";

  Serial.println("valeur mouv : ");
  Serial.println(mouv);
  Serial.println();


  // Sound sensor
  int sensorValue = analogRead(SOUND_SENSOR);//use A0 to read the electrical signal
  Serial.print("sensorValue : ");
  Serial.println(sensorValue);

  if(sensorValue > 350){
    sound = 1;
  } else {
    sound = 0;
  }

  Serial.println("sound : ");
  Serial.println(sound);
  Serial.println();

  if ( (mouv ==1) && (sound == 1)){
    activeAlarme = 1;
  }

  Serial.println("activeAlarme : ");
  Serial.println(activeAlarme);
  Serial.println();
    
  Serial.print("Sending message to topic: ");
  Serial.println(topic); 

  /*ENVOIE JSON */

  StaticJsonDocument<200> doc;
  doc["alarme"] = "activation de l'alarme";
  JsonArray data = doc.createNestedArray("data");
  data.add(activeAlarme);

  String message;
  serializeJson(doc, message);
    
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();

  Serial.println("message :" );
  Serial.println(message);
  Serial.println("fin message");
  Serial.println();

  }
}


//TRAITEMENT RECEPTION COMMANDES 

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  String messageRECU = "";

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    messageRECU += (char)mqttClient.read();    
  }

  Serial.println("messageRECU : ");
  Serial.println(messageRECU);

  //RECEPTION JSON
  StaticJsonDocument<200> doc;
  deserializeJson(doc, messageRECU);

  JsonObject obj = doc.as<JsonObject>();

  reception_ard2 = obj["data_arduino2"][0];
  Serial.print("reception_ard2: ");
  Serial.println(reception_ard2);
  Serial.println();

  if (reception_ard2 == 2){ // activation de l'alarme
    digitalWrite(LED,HIGH);
  }else if (reception_ard2 == 3){ // desactivation de l'alarme
    digitalWrite(LED,LOW);
  }
 
}
