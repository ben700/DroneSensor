#include "DroneFlasher.h"

DroneFlasher::DroneFlasher(DroneState __state)
{
  pinMode(LED_BUILTIN, OUTPUT);
  update(__state);
  start();
}

DroneFlasher::~DroneFlasher(){stop();}

void DroneFlasher::on(){digitalWrite(LED_BUILTIN, LOW);}
void DroneFlasher::off(){digitalWrite(LED_BUILTIN, HIGH);}
void DroneFlasher::blip(int __blips) {this->_blips = __blips;}

void DroneFlasher::stop() {
  _flipper.detach();
}


void DroneFlasher::update(DroneState __state){
  this->_state = __state;
  start();
}

void DroneFlasher::flipFlop() {
  int state = digitalRead(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, !state);
 // state ? Serial.print("flip :") : Serial.print("flop :");
 // printState();  
  state ? flip() : flop ();
}

void DroneFlasher::start() {
  switch (this->_state){
  case DroneState::SETUP:
    blip(5);
    break;    
  case DroneState::CONFIG:
    blip(5);
    break;
  case DroneState::OPS:
    blip(5);
    break;
  default:
    blip(5);
  }
  flop();
 }
 
void DroneFlasher::printState() {
  stop();
  if (_blips > 0){
    Serial.print("DroneState::_blips = " + String(_blips) + " ");
  }
  switch (this->_state){
  case DroneState::SETUP:
    Serial.println("DroneState::SETUP");
    break;        
  case DroneState::CONFIG:
    Serial.println("DroneState::CONFIG");
    break;
  case DroneState::OPS:
    Serial.println("DroneState::OPS");
    break;
  default:
    Serial.println("DroneState::default");
  }
}


void DroneFlasher::flip() {
  stop();
  if (_blips > 0){
    _flipper.attach(0.1, std::bind(&DroneFlasher::flipFlop, this) );
    _blips--;
  }
  else
  {
    switch (this->_state){
    case DroneState::SETUP:
      _flipper.attach(1, std::bind(&DroneFlasher::flipFlop, this) );
      break;      
    case DroneState::CONFIG:
      _flipper.attach(1, std::bind(&DroneFlasher::flipFlop, this) );
      break;
    case DroneState::OPS:
      break;
    default:
      _flipper.attach(0.1, std::bind(&DroneFlasher::flipFlop, this) );
    }
  }
}

void DroneFlasher::flop() {
  stop();
  if (_blips > 0){
    _flipper.attach(0.1, std::bind(&DroneFlasher::flipFlop, this) );
  
  }else
  {
    switch (this->_state){
    case DroneState::SETUP:
      _flipper.attach(5, std::bind(&DroneFlasher::flipFlop, this) );
      break;  
    case DroneState::CONFIG:
      _flipper.attach(1, std::bind(&DroneFlasher::flipFlop, this) );
      break;
    case DroneState::OPS:
      break;
    default:
      _flipper.attach(0.1, std::bind(&DroneFlasher::flipFlop, this) );
      }
  }
}
