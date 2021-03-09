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


    Ezo_board RTD = Ezo_board(102, "temperature");    //create an RTD circuit object who's address is 102 and name is "RTD"
    Ezo_board PH = Ezo_board(99, "PH");       //create a PH circuit object, who's address is 99 and name is "PH"
    Ezo_board EC = Ezo_board(100, "conductivity");      //create an EC circuit object who's address is 100 and name is "EC"
    Ezo_board DO = Ezo_board(97, "DO");    //create a DO circuit object who's address is 97 and name is "DO"
    Ezo_board ORP = Ezo_board(98, "oxidationReductionPotential");    //create a DO circuit object who's address is 97 and name is "DO"
    Ezo_board CO2 = Ezo_board(105, "CO2");    //create a DO circuit object who's address is 97 and name is "DO"
    Ezo_board HUM = Ezo_board(111, "humidity");    //create a DO circuit object who's address is 97 and name is "DO"
    
    //enable pins for each circuit
    const uint32_t EN_PH = 14;
    const uint32_t EN_EC = 12;
    const uint32_t EN_RTD = 15;
    const uint32_t EN_AUX = 13;

  
    const uint32_t short_delay = 300;              //how long we wait for most commands and queries
    const uint32_t long_delay = 1200;              //how long we wait for commands like cal and R (see datasheets for which commands have longer wait times)
    const uint32_t reading_delay = 2000;

    float k_val = 0;                                    //holds the k value of the ec circuit

  public:
    //array of ezo boards, add any new boards in here for the commands to work with them
    EZODevice device_list[4];
    //gets the length of the array automatically so we dont have to change the number every time we add new boards
    int device_list_len;

    
    DroneSensor (String __deviceMAC, String __deviceIP, String __deviceID, bool _DroneSensor_debug);

    void select_delay(String &str);
    void print_error_type(Ezo_board &Device, const char* success_string);
    String return_error_type(Ezo_board &Device, const char* success_string);
    String return_error_type(Ezo_board &Device, String success_string);
    void get_ec_k_value();
    void print_device_info(Ezo_board &Device);
    void receive_reading(Ezo_board &Device);
    bool processCommand(StaticJsonDocument<DOC_SIZE>& _command);
    bool processConfig(StaticJsonDocument<DOC_SIZE>& _config);
    String sensorPayload(String _EpochTime);
    void singleDeviceStatePayload (Ezo_board &Device, StaticJsonDocument<DOC_SIZE>& doc);
    String deviceStatePayload ();
    String calibrationCommands(String calibrationCommandError = "");
    void list_devices();
    String bootPayload(String _EpochTime);
    void debug();
    bool hasDevice();
    void turnParametersOn();
    void turnParametersOff();
    void setFallbackTemp(float __FallbackTemp);
    uint32_t next_step_time = 0;
    void test();
    int pollDelay = 60000;
    bool loggingData = true;
  private:
    String printEZOReadingStep(enum EZOReadingStep __currentStep);
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
