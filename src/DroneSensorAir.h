#ifndef DroneSensor_h
#define DroneSensor_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define NotConnected "Not Connected"


#define DOC_SIZE 1000
#include <stdint.h>
#include <Ezo_i2c.h>
#include <ArduinoJson.h>
#include <Wire.h>

enum EZOReadingStep {REQUEST_TEMP, READ_TEMP_AND_REQUEST_DEVICES, READ_RESPONSE, NO_DEVICES };


enum class EZOStatus {
    Connected,
    Unconnected
};


typedef struct{
    String _displayName;
    String _payloadName;
    uint32_t _precision;
} EZOParameter;

typedef struct
{
  String displayName;
  Ezo_board device;
  EZOStatus _status;
  bool tempCompensation;
  uint32_t   _countParameter;
  EZOParameter   _parameterList[4];
}  EZODevice;


class DroneSensor {


    Ezo_board CO2 = Ezo_board(105, "CO2");    //create a DO circuit object who's address is 97 and name is "DO"
    Ezo_board HUM = Ezo_board(111, "humidity");    //create a DO circuit object who's address is 97 and name is "DO"
    EZOParameter co2_CO2 = {"CO2", "CO2", 0};
    EZOParameter tem_CO2= {"Temperature", "thermalEquilibriumTemperature", 1};

    EZOParameter hum_HUM= {"Humitity", "humidity", 0};
    EZOParameter tem_HUM= {"Temperature", "temperature", 0};
    EZOParameter unk_HUM= {"Spacer", "", 0}; 
    EZOParameter dew_HUM= {"Dew Point", "dewPoint", 0};                       

    EZODevice CO2Item = (EZODevice) {"Gaseous CO2", CO2, EZOStatus::Unconnected, false, 2, {co2_CO2, tem_CO2}};
    EZODevice HUMItem = (EZODevice) {"Humitity", HUM, EZOStatus::Unconnected, false, 4, {hum_HUM, tem_HUM, unk_HUM, dew_HUM}};
    
    const uint32_t short_delay = 300;              //how long we wait for most commands and queries
    const uint32_t long_delay = 1200;              //how long we wait for commands like cal and R (see datasheets for which commands have longer wait times)
    const uint32_t reading_delay = 2000;


  public:
    //array of ezo boards, add any new boards in here for the commands to work with them
    EZODevice device_list[2] = {
      CO2Item,
     HUMItem  
    };
    //gets the length of the array automatically so we dont have to change the number every time we add new boards
    const uint8_t device_list_len = sizeof(device_list) / sizeof(device_list[0]);

    
    DroneSensor (String __deviceMAC, String __deviceIP, String __deviceID, bool _DroneSensor_debug);

    void select_delay(String &str);
    bool processCommand(StaticJsonDocument<DOC_SIZE>& _command);
    bool processConfig(StaticJsonDocument<DOC_SIZE>& _config);
    String sensorPayload(String _EpochTime);
    void singleDeviceStatePayload (Ezo_board &Device, StaticJsonDocument<DOC_SIZE>& doc);
    String deviceStatePayload ();
    String bootPayload(String _EpochTime);
    void debug();
    bool hasDevice();
    void turnParametersOn();
    void turnParametersOff();
    uint32_t next_step_time = 0;

    int pollDelay = 60000;
    bool loggingData = true;
  private:
     String lookupLedStatus(String LED);
    String lookupRestartCodes(String restartCodes);
    void sendReadCommand(StaticJsonDocument<DOC_SIZE>& _doc);
    bool headerPayload(StaticJsonDocument<DOC_SIZE>& _doc);
    bool parametersOn = false;
    String _deviceMAC;
    String _deviceIP;
    String _deviceID;
    bool DroneSensor_debug=false;
    enum EZOReadingStep current_step = NO_DEVICES;
    float _FallbackTemp = 19.5;

};


#endif
