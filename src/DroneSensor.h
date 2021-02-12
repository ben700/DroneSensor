#ifndef DroneSensor_h
#define DroneSensor_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define NotConnected "Not Connected"
#define DroneSensor_debug  false //true
#define DroneSensor_FallbackTemp 19.5
#define DOC_SIZE 1000
#include <stdint.h>
#include <Ezo_i2c.h>
#include <ArduinoJson.h>
#include <Wire.h>


enum class LedStatus: int {
    On = 0,
    Off = 1
};

enum class EZOStatus: int {
    Connected = 1,
    Unconnected = 0
};

/*typedef struct
{
  Ezo_board device;
  EZOStatus _status = EZOStatus::Unconnected;
}  EZODevice;
*/

class DroneSensor {


    Ezo_board PH = Ezo_board(99, "pH");       //create a PH circuit object, who's address is 99 and name is "PH"
    Ezo_board EC = Ezo_board(100, "Conductivity");      //create an EC circuit object who's address is 100 and name is "EC"
    Ezo_board RTD = Ezo_board(102, "Temperature");    //create an RTD circuit object who's address is 102 and name is "RTD"
    Ezo_board DO = Ezo_board(97, "Dissolved Oxygen");    //create a DO circuit object who's address is 97 and name is "DO"
    Ezo_board ORP = Ezo_board(98, "Oxidation Reduction Potential");    //create a DO circuit object who's address is 97 and name is "DO"

    //array of ezo boards, add any new boards in here for the commands to work with them
    Ezo_board device_list[5] = {
      PH,
      EC,
      RTD,
      DO,
      ORP
    };



    //enable pins for each circuit
    const int EN_PH = 14;
    const int EN_EC = 12;
    const int EN_RTD = 15;
    const int EN_AUX = 13;

    Ezo_board* default_board = &device_list[0]; //used to store the board were talking to

    //gets the length of the array automatically so we dont have to change the number every time we add new boards
    const uint8_t device_list_len = sizeof(device_list) / sizeof(device_list[0]);

    const unsigned long short_delay = 300;              //how long we wait for most commands and queries
    const unsigned long long_delay = 1200;              //how long we wait for commands like cal and R (see datasheets for which commands have longer wait times)
    const unsigned long reading_delay = 2000;

    float k_val = 0;                                    //holds the k value of the ec circuit




  public:
    DroneSensor (String __deviceMAC, String __deviceIP);

    void select_delay(String &str);
    void print_error_type(Ezo_board &Device, const char* success_string);
    String return_error_type(Ezo_board &Device, const char* success_string);
    String return_error_type(Ezo_board &Device, String success_string);
    void get_ec_k_value();
    void print_help();
    void print_device_info(Ezo_board &Device);
    void print_device_response(Ezo_board &Device);
    void receive_reading(Ezo_board &Device);
    String sensorPayload(String _EpochTime);

    String singleDeviceStatePayload (Ezo_board &Device);
    String deviceStatePayload ();
    String calibrationCommands(String calibrationCommandError = "");
    String tempStatePayload();
    String phStatePayload();
    String ecStatePayload();
    void list_devices();
    String readTemp();
    String readPH();
    String readEC();
    String readDO();
  private:
    String lookupLedStatus(String LED);
    String lookupRestartCodes(String restartCodes);
    void sendReadCommand(StaticJsonDocument<DOC_SIZE>& _doc);
    bool headerPayload(StaticJsonDocument<DOC_SIZE>& _doc);
    String _deviceMAC;
    String _deviceIP;
};


#endif