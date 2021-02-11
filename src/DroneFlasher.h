#ifndef DroneFlipper_h
#define DroneFlipper_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <stdint.h>
#include <Ticker.h>


enum class DroneState: int {
    SETUP = 0,
    CONFIG = 1,
    OPS = 2,
    ERR = 3
};

class DroneFlasher {

  public:
    DroneFlasher (DroneState __state);
    ~DroneFlasher ();
    void update(DroneState __state);
    void on();
    void off();
    void flipFlop();
    void blip(int __blips);
    void stop();
    void start();
    void printState();
  private:   
    void flip();
    void flop();
    Ticker _flipper;
    DroneState _state;
    int _blips =0;
};



#endif
