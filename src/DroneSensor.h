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



class DroneParameter {
  
    public:
    DroneParameter(char* __displayName, char*  __payloadName, int __precision);
    ~DroneParameter();
    
    char* _displayName;
    char*  _payloadName;
    int _precision; 
    
};

class DroneDevice {
    public:
    
    char* displayName;
    Ezo_board* device;
    EZOStatus _status;
    bool tempCompensation;
    int   _countParameter;
    DroneParameter*   _parameterList[];
    
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
  EZOParameter   _parameterList[];
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
    EZOParameter parameter_rtd[1] = {t_RTD}; 
    
    EZOParameter ph_PH = {"pH", "PH", 2};
                                         
    EZOParameter ec_EC = {"Conductivity", "conductivity", 0};
    EZOParameter tds_EC = {"Total Dissolved Solids", "totalDissolvedSolids", 0};
    EZOParameter sal_EC = {"Salinity", "salinity", 0};
    EZOParameter sg_EC = {"Specific Gravity", "specificGravity", 0};

    EZOParameter do_DO = {"Dissolved Oxygen", "DO", 0};
    EZOParameter sat_DO = {"Saturation", "saturation", 0};

    EZOParameter orp_ORP = {"Oxidation Reduction Potential", "oxidationReductionPotential", 0};

    EZOParameter co2_CO2 = {"CO2", "CO2", 0};
    EZOParameter tem_CO2= {"Temperature", "thermalEquilibriumTemperature", 1};

    EZOParameter hum_HUM= {"Humitity", "humidity", 0};
    EZOParameter tem_HUM= {"Temperature", "temperature", 0};
    EZOParameter unk_HUM= {"Spacer", "", 0}; 
    EZOParameter dew_HUM= {"Dew Point", "dewPoint", 0};                       

                                         
            
    EZODevice RTDItem =(EZODevice) {"Temperature", RTD, EZOStatus::Unconnected, false, 1, parameter_rtd};
    EZODevice ECItem = (EZODevice) {"Conductivity", EC, EZOStatus::Unconnected, true, 4, {ec_EC, tds_EC, sal_EC, sg_EC}};
    EZODevice PHItem = (EZODevice) {"pH", PH, EZOStatus::Unconnected, false, 1, {ph_PH}};
    EZODevice DOItem = (EZODevice) {"Dissolved Oxygen", DO, EZOStatus::Unconnected, true, 2, {do_DO, sat_DO}};
    EZODevice ORPItem = (EZODevice) {"Oxidation Reduction Potential", ORP, EZOStatus::Unconnected, true, 0, {orp_ORP}};
    EZODevice CO2Item = (EZODevice) {"Gaseous CO2", CO2, EZOStatus::Unconnected, false, 2, {co2_CO2, tem_CO2}};
    EZODevice HUMItem = (EZODevice) {"Humitity", HUM, EZOStatus::Unconnected, false, 4, {hum_HUM, tem_HUM, unk_HUM, dew_HUM}};
  



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
    EZODevice device_list[7] = {
      RTDItem,
      ECItem,
      PHItem,
      DOItem,
      ORPItem,
      CO2Item,
      HUMItem  
    };
    //gets the length of the array automatically so we dont have to change the number every time we add new boards
    const uint8_t device_list_len = sizeof(device_list) / sizeof(device_list[0]);

    
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
