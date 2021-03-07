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


typedef struct{
    String _displayName;
    String _payloadName;
    int _precision;
} EZOParameter;

typedef struct
{
  String displayName;
  Ezo_board device;
  EZOStatus _status;
  int _precision;
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

    EZOParameter t_RTD = {"Temperature", "temperature", 1};
   // EZOParameter rtd_parameterList[1] = {t_RTD};
 
    EZOParameter ph_PH = {"pH", "PH", 2};
    EZOParameter ph_parameterList[1] = {ph_PH};
                                         
    EZOParameter ec_EC = {"Conductivity", "conductivity", 0};
    EZOParameter tds_EC = {"Total Dissolved Solids", "totalDissolvedSolids", 0};
    EZOParameter sal_EC = {"Salinity", "salinity", 0};
    EZOParameter sg_EC = {"Specific Gravity", "specificGravity", 0};
    EZOParameter ec_parameterList[4] = {ec_EC, tds_EC, sal_EC, sg_EC};

    EZOParameter do_DO = {"Dissolved Oxygen", "DO", 0};
    EZOParameter sat_DO = {"Saturation", "saturation", 0};
    EZOParameter do_parameterList[2] = {do_DO, sat_DO};

    EZOParameter orp_ORP = {"Oxidation Reduction Potential", "oxidationReductionPotential", 0};
    EZOParameter orp_parameterList[1] = {orp_ORP};

    EZOParameter co2_CO2 = {"CO2", "CO2", 0};
    EZOParameter tem_CO2= {"Temperature", "temperature", 1};
    EZOParameter co2_parameterList[2] = {co2_CO2, tem_CO2};

    EZOParameter hum_HUM= {"Humitity", "humidity", 0};
    EZOParameter tem_HUM= {"Temperature", "temperature", 0};
    EZOParameter dew_HUM= {"Dew Point", "dewPoint", 0};                       
    EZOParameter hum_parameterList[3] = {hum_HUM, tem_HUM, dew_HUM};

                                         
            
    EZODevice RTDItem =(EZODevice) {"Temperature", RTD, EZOStatus::Unconnected, 1, {t_RTD}};
    EZODevice ECItem = (EZODevice) {"Conductivity", EC, EZOStatus::Unconnected, 0, ec_parameterList};
    EZODevice PHItem = (EZODevice) {"pH", PH, EZOStatus::Unconnected, 2, ph_parameterList};
    EZODevice DOItem = (EZODevice) {"Dissolved Oxygen", DO, EZOStatus::Unconnected, 1, do_parameterList};
    EZODevice ORPItem = (EZODevice) {"Oxidation Reduction Potential", ORP, EZOStatus::Unconnected, 0, orp_parameterList};
    EZODevice CO2Item = (EZODevice) {"Gaseous CO2", CO2, EZOStatus::Unconnected, 0, co2_parameterList};
    EZODevice HUMItem = (EZODevice) {"Humitity", HUM, EZOStatus::Unconnected, 0, hum_parameterList};
  
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
