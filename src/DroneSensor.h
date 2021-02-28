#ifndef DroneSensor_h
#define DroneSensor_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define NotConnected "Not Connected"


#define DroneSensor_FallbackTemp 19.5
#define DOC_SIZE 1000
#include <stdint.h>
#include <Ezo_i2c.h>
#include <ArduinoJson.h>
#include <Wire.h>

enum EZOReadingStep {REQUEST_TEMP, READ_TEMP_AND_REQUEST_DEVICES, READ_RESPONSE, NO_DEVICES };

enum class LedStatus: int {
    On = 0,
    Off = 1
};

enum class EZOStatus: int {
    Connected = 1,
    Unconnected = 0
};

typedef struct
{
  String displayName;
  Ezo_board device;
  EZOStatus _status;
  int _precision;
}  EZODevice;


class DroneSensor {


    Ezo_board RTD = Ezo_board(102, "temperature");    //create an RTD circuit object who's address is 102 and name is "RTD"
    Ezo_board PH = Ezo_board(99, "PH");       //create a PH circuit object, who's address is 99 and name is "PH"
    Ezo_board EC = Ezo_board(100, "conductivity");      //create an EC circuit object who's address is 100 and name is "EC"
    Ezo_board DO = Ezo_board(97, "DO");    //create a DO circuit object who's address is 97 and name is "DO"
    Ezo_board ORP = Ezo_board(98, "oxidationReductionPotential");    //create a DO circuit object who's address is 97 and name is "DO"

    EZODevice RTDItem = {"Temperature", RTD, EZOStatus::Unconnected, 1};
    EZODevice ECItem = {"Conductivity", EC, EZOStatus::Unconnected, 0};
    EZODevice PHItem = {"pH", PH, EZOStatus::Unconnected, 2};
    EZODevice DOItem = {"Dissolved Oxygen", DO, EZOStatus::Unconnected, 1};
    EZODevice ORPItem = {"Oxidation Reduction Potential", ORP, EZOStatus::Unconnected, 0};
  
    //array of ezo boards, add any new boards in here for the commands to work with them
    EZODevice device_list[5] = {
      RTDItem,
      ECItem,
      PHItem,
      DOItem,
      ORPItem
    };


    //enable pins for each circuit
    const int EN_PH = 14;
    const int EN_EC = 12;
    const int EN_RTD = 15;
    const int EN_AUX = 13;

    Ezo_board* default_board = &device_list[0].device; //used to store the board were talking to

    //gets the length of the array automatically so we dont have to change the number every time we add new boards
    const uint8_t device_list_len = sizeof(device_list) / sizeof(device_list[0]);

    const unsigned long short_delay = 300;              //how long we wait for most commands and queries
    const unsigned long long_delay = 1200;              //how long we wait for commands like cal and R (see datasheets for which commands have longer wait times)
    const unsigned long reading_delay = 2000;

    float k_val = 0;                                    //holds the k value of the ec circuit




  public:
    DroneSensor (String __deviceMAC, String __deviceIP, String __deviceID, bool _DroneSensor_debug);

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
    void sensorPayloadAsyc(String _EpochTime, StaticJsonDocument<DOC_SIZE>& _doc);
    String singleDeviceStatePayload (Ezo_board &Device);
    void singleDeviceStatePayloadAsyc (EZODevice &Device, StaticJsonDocument<DOC_SIZE>& _doc);
    String deviceStatePayload ();
    void deviceStatePayloadAsyc (StaticJsonDocument<DOC_SIZE>& _doc);
    String calibrationCommands(String calibrationCommandError = "");
    String tempStatePayload();
    String phStatePayload();
    String ecStatePayload();
    void list_devices();
    String readTemp();
    String readPH();
    String readEC();
    String readDO();
    String readORP();
    String bootPayload(String _EpochTime);
    enum EZOReadingStep buildDeviceStatePayload(StaticJsonDocument<DOC_SIZE>& _doc);
    void debug();
    bool hasDevice();
    void turnParametersOn();
    uint32_t next_step_time = 0;
  private:
    String printEZOReadingStep(enum EZOReadingStep __currentStep);
    String lookupLedStatus(String LED);
    String lookupRestartCodes(String restartCodes);
    void sendReadCommand(StaticJsonDocument<DOC_SIZE>& _doc);
    bool headerPayload(StaticJsonDocument<DOC_SIZE>& _doc);
    String _deviceMAC;
    String _deviceIP;
    String _deviceID;
    bool DroneSensor_debug=false;
    enum EZOReadingStep current_step = NO_DEVICES;
};


#endif
