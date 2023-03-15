#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQUnifiedsensor.h>
#include <WiFiManager.h>
// GPIO 5 D1
//#define LED D4

// WiFi
//const char *ssid = "Test"; // Enter your WiFi name
//const char *password = "062340161";  // Enter WiFi password

// MQTT Broker
//const char *client_id = "esp8266-client";
const char *mqtt_broker = "broker.emqx.io";
const char *deviceID = "device1";
const int mqtt_port = 1883;
const char* gas_state_topic = "gas_detect/device1/gas_state";
const char* relay_state_topic = "gas_detect/device1/relay_state";
const char* gas_value_topic = "gas_detect/device1/gas_value";
const char* adc_value_topic = "gas_detect/device1/adc";
const char* relay_control_topic = "gas_detect/device1/relay_control";

//Definitions
#define placa "ESP8266"
#define Voltage_Resolution 3.3
#define pin A0 //Analog input 0 of your arduino
#define type "MQ-7" //MQ5
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ7CleanAir 27.5  //RS / R0 = 6.5 ppm 
#define redLed D6
#define greenLed D8
#define Relay D2
#define _A 700000000
#define _B -7.703

MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);
int adc,relayState;
float v,R0,RL,RS,ratio,gasValue;
float calcR0 = 0;
String Status = "UnLock";
boolean relay_state = true; 
boolean Sensor_State = true; 
int valve = 0;
String relay_t = "";
String Sensor_msg = "";

WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    pinMode(redLed, INPUT);
    pinMode(greenLed, OUTPUT);
    pinMode(Relay, OUTPUT);
    //pinMode(LED,OUTPUT);
    // connecting to a WiFi network
    //WiFi.begin(ssid, password);
    wifiManager.autoConnect("Gas_detect");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println(".");
    }
    Serial.println("Connected to the WiFi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        
        //client_id += String(WiFi.macAddress());
        Serial.println("Connecting to public emqx mqtt broker.....");
        if (client.connect(deviceID)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    Serial.print("Calibrating please wait.");
    for(int i = 1; i<=10; i ++)
    {
      MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
      calcR0 += MQ7.calibrate(RatioMQ7CleanAir);
      Serial.print(".");
    }
    MQ7.setR0(calcR0/10);
    Serial.println("  done!.");
    
    if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
    if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
    
    //subscribe
    client.subscribe(relay_control_topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    String message;
    for (int i = 0; i < length; i++) {
        message = message + (char) payload[i];  // convert *byte to string
    }
    Serial.print(message);
    if (message == "on") { digitalWrite(Relay, LOW); }   // LED on
    if (message == "off") { digitalWrite(Relay, HIGH); } // LED off
    Serial.println();
    Serial.println("-----------------------");
}

void loop() {
    calculate_gas();
    state();
    //if (!client.connected()) {
    //mqttReConnect();
  //}
    client.loop();
    client.publish(gas_state_topic, String(Sensor_msg).c_str());
    client.publish(relay_state_topic, String(relay_t).c_str());
    client.publish(adc_value_topic,String(adc).c_str());
    client.publish(gas_value_topic, String(gasValue).c_str());
    //client.disconnect();
    delay(1000);
    wifireconnect();
        
}

void wifireconnect(){
  while ( WiFi.status() != WL_CONNECTED){
    delay(500);
    ESP.restart();
    }
  }

void calculate_gas(){
  int led_state = 0 ;
  led_state = digitalRead(redLed);
  adc = analogRead(pin);
  v = adc*Voltage_Resolution/1023.00;
  R0 = MQ7.getR0();
  RL = MQ7.getRL();
  RS = MQ7.getRS(); 
  ratio = RS/R0;
  gasValue = _A*pow(ratio, _B);
  MQ7.update();
  Serial.print("ADC in :");
  Serial.println(adc);
  Serial.print("The concentrate gas :");
  Serial.print(gasValue);
  Serial.println("PPM");
  Serial.println(led_state);
  if (led_state == 1){
    digitalWrite(Relay,HIGH);
    Sensor_msg = "檢出瓦斯";
    Serial.println(Sensor_msg);
    }
  else{
    digitalWrite(Relay,LOW);
    Sensor_msg = "未檢出瓦斯";
    Serial.println(Sensor_msg);
    }
}
void state(){
  relayState = digitalRead(Relay);
  if (relayState == 1){    
    relay_t = "關瓦斯"; 
    }

  if (relayState == 0) {
    relay_t = "開瓦斯";
    }
  Serial.println(relay_t);
}
