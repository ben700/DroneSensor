#include "droneFlasher.h"



DroneFlasher *flasher;



void setup() {
  Serial.begin(115200);
  flasher = new DroneFlasher(DroneState::SETUP);
}

void loop() {
delay(10000);
Serial.println("OPS");
flasher->update(DroneState::OPS);

delay(30000);
Serial.println("CONFIG");
flasher->update(DroneState::CONFIG);
delay(30000);
Serial.println("ERR");
flasher->update(DroneState::ERR);
delay(30000);
Serial.println("SETUP");
flasher->update(DroneState::SETUP);


  
}
