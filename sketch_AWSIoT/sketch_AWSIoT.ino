
#include <MQTT.h>

#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "dt/europe/FR/Octank_ESP32_EUR_FR_MRS/sensor-value"
#define AWS_IOT_SUBSCRIBE_TOPIC "dt/europe/FR/Octank_ESP32_EUR_FR_MRS/sensor-value"

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_SHADOW_TOPIC   "$aws/things/Octank_ESP32_EUR_FR_MRS/shadow/update"
#define AWS_IOT_SUBSCRIBE_SHADOW_TOPIC "$aws/things/Octank_ESP32_EUR_FR_MRS/shadow/update"

#include <M5StickC.h>
#include <Wire.h>

// MLX90614       M5stick

// Vin            3.3V
// GND            GND
// SCL            G26
// SDA            G0


WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
unsigned long lastMillis = 0;
unsigned long mqttPushTime = 360000; 


uint16_t result;
float temperature;

void connectAWS()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}


void publishShadowMessage()
{

  DynamicJsonDocument docGeo(1024);
  DynamicJsonDocument docReported(1024);
  DynamicJsonDocument docState(1024);
  JsonObject geoObject  = docGeo.createNestedObject("geo");
  geoObject["latitude"] = "43.2835907";
  geoObject["longitude"] = "5.3607719";

  JsonObject reportedObject  = docReported.createNestedObject("reported");
  reportedObject["name"] = "Octank_ESP32_EUR_FR_MRS";
  reportedObject["enabled"] = "true";
  reportedObject["geo"] = geoObject;

  JsonObject stateObject  = docState.createNestedObject("state");
  stateObject["reported"] = reportedObject;


  //StaticJsonDocument<200> doc;
  //doc["time"] = millis();
  //doc["time"."time"] = millis();
  //doc["sensor_a0"] = analogRead(0);
  char jsonBuffer[512];
  serializeJson(docState, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_SHADOW_TOPIC, jsonBuffer);

      Serial.println("Publish message : topic : ");
  Serial.println(AWS_IOT_SUBSCRIBE_SHADOW_TOPIC);
  Serial.println(jsonBuffer);
  
}

void initTemp(){
  M5.begin();
  
  Wire.begin(0,26);
  Serial.begin(115200);

  M5.Lcd.setRotation(3);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
}

void publishMessage(float temperature)
{
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["pressure"] = "30";
  doc["timestamp"] = millis();
  doc["latitude"] = "43.2835907";
  doc["longitude"] = "5.3607719";
  doc["status"] = "1";

  
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
    Serial.println("Publish message : topic : ");
  Serial.println(AWS_IOT_PUBLISH_TOPIC);
  Serial.println(jsonBuffer);
}

void displayAndPushTemp(){

  Wire.beginTransmission(0x5A);          // Send Initial Signal and I2C Bus Address
  Wire.write(0x07);                      // Send data only once and add one address automatically.
  Wire.endTransmission(false);           // Stop signal
  Wire.requestFrom(0x5A, 2);             // Get 2 consecutive data from 0x5A, and the data is stored only.
  result = Wire.read();                  // Receive DATA
  result |= Wire.read() << 8;            // Receive DATA
  
  temperature = result * 0.02 - 273.15;
  
  M5.Lcd.fillRect(0,0,200,49,BLACK);
  M5.Lcd.setCursor(0, 30);

  
  M5.Lcd.print(temperature);
  M5.Lcd.print(" C");
  Serial.println(result);

 if (M5.BtnA.wasReleased()) {
    M5.Lcd.fillRect(0,0,200,200,BLACK);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.print(temperature);
    M5.Lcd.print(" C (Mem)");
    
 }

 if (M5.BtnB.wasReleased()) {

    M5.Lcd.fillRect(0,0,200,200,BLACK);
   
 }

  // publish a message roughly every second.
  if (millis() - lastMillis > mqttPushTime) {
    lastMillis = millis();
    publishMessage(temperature);
  }
  delay(500);
  M5.update();

}


void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  Serial.begin(9600);
  connectAWS();
  publishShadowMessage();
  initTemp();
}

void loop() {
  
  client.loop();

  displayAndPushTemp();
 
}
