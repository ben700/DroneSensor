#include <DroneSensor.h>
#include <ESP8266WiFi.h> 

#define DroneSensor_debug false

DroneSensor *sensors;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  sensors = new DroneSensor(WiFi.macAddress(), WiFi.localIP().toString(), "DeviceID");

}

void loop() {
  
  Serial.println("Testing...");
  sensors->list_devices();

  sensors->bootPayload("Time");
 
  sensors->deviceStatePayload();
  sensors->sensorPayload("Time");
  sensors->calibrationCommands();

  delay(10000);
}
