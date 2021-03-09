#include "DroneSensor.h"

DroneSensor::DroneSensor(String __deviceMAC, String __deviceIP, String __deviceID, bool _DroneSensor_debug = false)
{
  //DroneParameter* test = new DroneParameter("Temperature", "temperature", 1);
  DroneSensor_debug = _DroneSensor_debug;
  this->_deviceMAC = __deviceMAC;
  this->_deviceIP = __deviceIP;
  this->_deviceID = __deviceID;
  Wire.begin(); 
  pinMode(EN_PH, OUTPUT);                                                         //set enable pins as outputs
  pinMode(EN_EC, OUTPUT);
  pinMode(EN_RTD, OUTPUT);
  pinMode(EN_AUX, OUTPUT);
  digitalWrite(EN_PH, LOW);                                                       //set enable pins to enable the circuits
  digitalWrite(EN_EC, LOW);
  digitalWrite(EN_RTD, HIGH);
  digitalWrite(EN_AUX, LOW); 
  if (DroneSensor_debug) { Serial.println(F("DroneSensor::DroneSensor()"));}
  delay(long_delay);
  int i = 0;
  byte address = RTD.get_address();
  Wire.beginTransmission(address);
  if (!Wire.endTransmission()){
    this->current_step = EZOReadingStep::REQUEST_TEMP;
    EZOParameter t_RTD = {"Temperature", "temperature", 1};
    EZODevice RTDItem =(EZODevice) {"Temperature", RTD, EZOStatus::Unconnected, false, 1, {t_RTD}};
    this->device_list[i++] = RTDItem;  
    if (DroneSensor_debug) {
        Serial.println(F("EZO Circuit Temperature found at address 102"));
    }
  }
  
  address = PH.get_address();
  Wire.beginTransmission(address);
  if (!Wire.endTransmission()){
    this->current_step = EZOReadingStep::REQUEST_TEMP;
    EZOParameter ph_PH = {"pH", "PH", 2};
    EZODevice PHItem = (EZODevice) {"pH", PH, EZOStatus::Unconnected, false, 1, {ph_PH}};
    this->device_list[i++] = PHItem;  
    if (DroneSensor_debug) {
        Serial.println(F("EZO Circuit PH found at address 99"));
    }
  }
  
  address = EC.get_address();
  Wire.beginTransmission(address);
  if (!Wire.endTransmission()){
    this->current_step = EZOReadingStep::REQUEST_TEMP;
    EZOParameter ec_EC = {"Conductivity", "conductivity", 0};
    EZOParameter tds_EC = {"Total Dissolved Solids", "totalDissolvedSolids", 0};
    EZOParameter sal_EC = {"Salinity", "salinity", 0};
    EZOParameter sg_EC = {"Specific Gravity", "specificGravity", 0};
    EZODevice ECItem = (EZODevice) {"Conductivity", EC, EZOStatus::Unconnected, true, 4, {ec_EC, tds_EC, sal_EC, sg_EC}};
    this->device_list[i++] = ECItem;  
    if (DroneSensor_debug) {
        Serial.println(F("EZO Circuit EC found at address 100"));
    }
  }
  
  address = DO.get_address();
  Wire.beginTransmission(address);
  if (!Wire.endTransmission()){
    this->current_step = EZOReadingStep::REQUEST_TEMP;
    EZOParameter do_DO = {"Dissolved Oxygen", "DO", 0};
    EZOParameter sat_DO = {"Saturation", "saturation", 0};
    EZODevice DOItem = (EZODevice) {"Dissolved Oxygen", DO, EZOStatus::Unconnected, true, 2, {do_DO, sat_DO}};
    this->device_list[i++] = DOItem;  
    if (DroneSensor_debug) {
        Serial.println(F("EZO Circuit DO found at address 97"));
    }
  }
  
  address = ORP.get_address();
  Wire.beginTransmission(address);
  if (!Wire.endTransmission()){
    this->current_step = EZOReadingStep::REQUEST_TEMP;
    EZOParameter orp_ORP = {"Oxidation Reduction Potential", "oxidationReductionPotential", 0};
    EZODevice ORPItem = (EZODevice) {"Oxidation Reduction Potential", ORP, EZOStatus::Unconnected, true, 0, {orp_ORP}};
    this->device_list[i++] = ORPItem;  
    if (DroneSensor_debug) {
        Serial.println(F("EZO Circuit ORP found at address 98"));
    }
  }
  
  
  address = CO2.get_address();
  Wire.beginTransmission(address);
  if (!Wire.endTransmission()){
    this->current_step = EZOReadingStep::REQUEST_TEMP;
    EZOParameter co2_CO2 = {"CO2", "CO2", 0};
    EZOParameter tem_CO2= {"Temperature", "thermalEquilibriumTemperature", 1};
    EZODevice CO2Item = (EZODevice) {"Gaseous CO2", CO2, EZOStatus::Unconnected, false, 2, {co2_CO2, tem_CO2}};
    this->device_list[i++] = CO2Item;  
    if (DroneSensor_debug) {
        Serial.println(F("EZO Circuit CO2 found at address 105"));
    }
  }
  
  
  address = HUM.get_address();
  Wire.beginTransmission(address);
  if (!Wire.endTransmission()){
    this->current_step = EZOReadingStep::REQUEST_TEMP;
    EZOParameter hum_HUM= {"Humitity", "humidity", 0};
    EZOParameter tem_HUM= {"Temperature", "temperature", 0};
    EZOParameter unk_HUM= {"Spacer", "", 0}; 
    EZOParameter dew_HUM= {"Dew Point", "dewPoint", 0};                       
    EZODevice HUMItem = (EZODevice) {"Humitity", HUM, EZOStatus::Unconnected, false, 4, {hum_HUM, tem_HUM, unk_HUM, dew_HUM}};
    this->device_list[i++] = HUMItem;  
    if (DroneSensor_debug) {
        Serial.println(F("EZO Circuit HUM found at address 111"));
    }
  }
  this->device_list_len = i;
  setFallbackTemp(this->_FallbackTemp);
}

void DroneSensor::setFallbackTemp(float __FallbackTemp){
  this->_FallbackTemp = __FallbackTemp;
  for (int i = 0; i < device_list_len; i++ )
  {
    if(device_list[i].tempCompensation){
      device_list[i].device.send_cmd_with_num("T,", this->_FallbackTemp);
    }
  }
}

void DroneSensor::debug() {
  DroneSensor_debug=true;
}
bool DroneSensor::hasDevice(){
 if (this->current_step == EZOReadingStep::NO_DEVICES){
   return false;
 }else{
   return true;
 }
}
// determines how long we wait depending on the command
void DroneSensor::select_delay(String &str) {
  if (str.indexOf("CAL") != -1 || str.indexOf("R") != -1) {
    delay(long_delay);
  } else {
    delay(short_delay);
  }
}
// used for printing either a success_string message if a command was successful or the error type if it wasnt
void DroneSensor::print_error_type(Ezo_board &Device, const char* success_string) {
      Serial.println(return_error_type(Device, success_string));   //the command was successful, print the success string
}

String DroneSensor::return_error_type(Ezo_board &Device, String success_string) {
  switch (Device.get_error()) 
  {
    case Ezo_board::SUCCESS:
      return (success_string);
    case Ezo_board::FAIL:
      return ("Failed");
    case Ezo_board::NOT_READY:
      return ("Pending");
    case Ezo_board::NOT_READ_CMD:
      return ("Not Read Command");
    case Ezo_board::NO_DATA:
      return ("No Data");
  }
  return ("No Device");
}


String DroneSensor::lookupRestartCodes(String restartCodes){
  if (restartCodes == "P"){
    //return "Powered Off";
    return "Power";
  }else if (restartCodes == "S"){
    return "Software Reset";
  }else if (restartCodes == "B"){
    return "Brown Out";
  }else if (restartCodes == "W"){
    return "Watchdog";
  }else if (restartCodes == "U"){
    return "Watchdog";
  }else{
    return "Unknowner";
  }
}

String DroneSensor::lookupLedStatus(String LED){
  if (LED == "1"){ return "On";}else{return "Off";}
}

String DroneSensor::return_error_type(Ezo_board &Device, const char* success_string) {
  switch (Device.get_error()) 
  {
    case Ezo_board::SUCCESS:
      return (success_string);
    case Ezo_board::FAIL:
      return ("Failed");
    case Ezo_board::NOT_READY:
      return ("Pending");
    case Ezo_board::NOT_READ_CMD:
      return ("Not Read Command");
    case Ezo_board::NO_DATA:
      return ("No Data");
  }
  return ("No Device");
}



void DroneSensor::get_ec_k_value(){                                    //function to query the value of the ec circuit
  char rx_buf[10];                                        //buffer to hold the string we receive from the circuit
  EC.send_cmd("k,?");                                     //query the k value
  delay(300);
  if(EC.receive_cmd(rx_buf, 10) == Ezo_board::SUCCESS){   //if the reading is successful
    k_val = String(rx_buf).substring(3).toFloat();        //parse the reading into a float
  }
}



// prints the boards name and I2C address
void DroneSensor::print_device_info(Ezo_board &Device) {
  Serial.print(Device.get_name());
  Serial.print(" ");
  Serial.print(Device.get_address());
}


void DroneSensor::turnParametersOn() {
  this->parametersOn = true;
  for (int i = 0; i < device_list_len; i++) {
    if(device_list[i].device.get_name() == "conductivity" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("Conductivity is "));}
      String command = "O,EC,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,TDS,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,S,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,SG,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == "DO" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) { Serial.print(F("Dissolved Oxygen is "));}
      String command = "O,mg,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,%,1";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == "CO2" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("CO2 is "));}
      String command = "O,t,1";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == "HUM" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("Humitity is "));}
      String command = "O,HUM,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,T,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,Dew,1";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }
  }
}
void DroneSensor::turnParametersOff() {
  this->parametersOn = false;
  for (int i = 0; i < device_list_len; i++) {
    if(device_list[i].device.get_name() == "conductivity" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("Conductivity is "));}
      String command = "O,EC,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,TDS,0";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,S,0";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,SG,0";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == "DO" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("Dissolved Oxygen is "));}
      String command = "O,mg,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,%,0";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == "CO2" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("CO2 is "));}
      String command = "O,t,0";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == "HUM" and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("Humitity is "));}
      String command = "O,HUM,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,T,0";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,Dew,0";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }
  }
}
void DroneSensor::receive_reading(Ezo_board &Device) {              // function to decode the reading after the read command was issued

  if (DroneSensor_debug) {Serial.print(Device.get_name()); Serial.print(F(": "));}
  Device.receive_read_cmd();              //get the response data and put it into the [Device].reading variable if successful

  if (DroneSensor_debug) {print_error_type(Device, String(Device.get_last_received_reading(), 2).c_str());}
}


void DroneSensor::sendReadCommand(StaticJsonDocument<DOC_SIZE>& _doc) {
  if (DroneSensor_debug) { Serial.println(F("DroneSensor::sendReadCommand()"));}
  float temp = this->_FallbackTemp;
  if(device_list[0]._status == EZOStatus::Connected){
    device_list[0].device.send_read_cmd();
    delay(reading_delay);
    receive_reading(device_list[0].device);
    if (DroneSensor_debug) { print_error_type(device_list[0].device, "Reading Temp Success");} 
    if ((device_list[0].device.get_error() == Ezo_board::SUCCESS) && (device_list[0].device.get_last_received_reading() > -1000.0))
    {
      temp =device_list[0].device.get_last_received_reading();
      if (DroneSensor_debug) { Serial.print(F("DroneSensor::sendReadCommand RTD.get_last_received_reading() ")); Serial.println(temp);}
    } 
  
    for (int i = 0; i < device_list_len; i++ )
    {
      if(device_list[i]._status == EZOStatus::Connected){
        if(device_list[i].tempCompensation){
          device_list[i].device.send_read_with_temp_comp(temp);
        }else{
          device_list[i].device.send_read_cmd();
        }
        if (DroneSensor_debug) { Serial.println("DroneSensor::sendReadCommand() -> Sent read command to " + String(device_list[i].device.get_name())); }  
      }
    }
  }else{
    for (int i = 0; i < device_list_len; i++ )
    {
      if(device_list[i]._status == EZOStatus::Connected){
        device_list[i].device.send_read_cmd();
        if (DroneSensor_debug) { Serial.println("DroneSensor::sendReadCommand() -> Sent read command to " + String(device_list[i].device.get_name())); }  
      }
    }
  }
}
String DroneSensor::printEZOReadingStep(enum EZOReadingStep __currentStep){
  switch(__currentStep)
  {
    case EZOReadingStep::REQUEST_TEMP:
      return "REQUEST_TEMP";
      break;
    case EZOReadingStep::READ_TEMP_AND_REQUEST_DEVICES:
      return "READ_TEMP_AND_REQUEST_DEVICES";
      break;
    case EZOReadingStep::READ_RESPONSE:
      return "READ_RESPONSE";
    case EZOReadingStep::NO_DEVICES:
      return "NO_DEVICES";
  }
}

String DroneSensor::sensorPayload(String _EpochTime)
{
  if (DroneSensor_debug) { Serial.println(F("DroneSensor::sensorPayload()"));  }
  StaticJsonDocument<DOC_SIZE> doc;
     
  doc["deviceTime"] = _EpochTime;

  headerPayload(doc);
  
  if (DroneSensor_debug) { Serial.println(F("Header ----------")); serializeJsonPretty(doc, Serial); }

  sendReadCommand(doc);
  delay(reading_delay);
  for (int i = 0; i < device_list_len; i++ )
  {
    if(device_list[i]._status == EZOStatus::Connected){
      char receive_buffer[32];
      if(device_list[i].device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
        char * pReading;
        char delimiter[] = ",";
        pReading = strtok (receive_buffer,delimiter);
        int countParameter = 1;
        if(this->parametersOn) { countParameter = device_list[i]._countParameter;}
        for(int y=0; y < countParameter; y++){
          if(pReading != NULL){
            if(device_list[i]._parameterList[y]._payloadName != NULL and device_list[i]._parameterList[y]._payloadName.length() >0){
              doc[device_list[i]._parameterList[y]._payloadName] = String(atof(pReading), device_list[i]._parameterList[y]._precision);
            }
          }else{
              if (DroneSensor_debug) {Serial.println("Error: Null but expected to get " + device_list[i]._parameterList[y]._displayName + " for " + String(device_list[i].device.get_name()));}
          }          
          pReading = strtok (NULL,delimiter);
        }
      }
    }
  }
  
  if(DroneSensor_debug ){serializeJsonPretty(doc, Serial);}
  String output;
  serializeJson(doc, output);
  return output; 
}

void DroneSensor::singleDeviceStatePayload (Ezo_board &Device, StaticJsonDocument<DOC_SIZE>& doc){

  String command = "I";
  String cmdReply;
  char receive_buffer[32];

  Device.send_cmd(command.c_str());
  select_delay(command);

  if(Device.receive_cmd(receive_buffer, 32) != Ezo_board::SUCCESS){   //if the reading is successful
    print_error_type(Device, "success_string"); 
    doc[Device.get_name()] = NotConnected;
  }else{
    cmdReply = String(receive_buffer);        //parse the reading into a float

    String type = cmdReply.substring(cmdReply.indexOf(",")+1, cmdReply.indexOf(",",4));
    String firm = cmdReply.substring(cmdReply.indexOf(",",4)+1);
  
    doc[Device.get_name()]["Name"] = Device.get_name();
    doc[Device.get_name()]["Firmware"] = firm;
  
    command = "CAL,?";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }
  
    String calibrationPoints = cmdReply.substring(cmdReply.indexOf("CAL,")+4);
    doc[Device.get_name()]["Calibration Points"] = calibrationPoints;
  //  doc[Device.get_name()]["CP"] = calibrationPoints;
  
    command = "Status";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }

    String reasonForRestart = cmdReply.substring(cmdReply.indexOf(",")+1,cmdReply.indexOf(",", cmdReply.indexOf(",")+1) );
    String VoltageatVcc = cmdReply.substring(cmdReply.indexOf(",", cmdReply.indexOf(",")+1)+1); 
    doc[Device.get_name()]["Restart"] = lookupRestartCodes(reasonForRestart);
    doc[Device.get_name()]["Vcc"] = VoltageatVcc;

    
    command = "L,?";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }

    String LED = cmdReply.substring(cmdReply.indexOf("L,")+2);
    doc[Device.get_name()]["LED"] = lookupLedStatus(LED);
  }
    
  return;
}


String DroneSensor::deviceStatePayload (){
  StaticJsonDocument<DOC_SIZE> doc;
  doc["Fallback Temperature"] = this->_FallbackTemp;
  doc["Poll Delay"] = this->pollDelay;
  this->parametersOn ? doc["Parameters"] = "On" : doc["Parameters"] = "Off" ;
  
  for (int i = 0; i < device_list_len; i++ )
  {
    if(device_list[i]._status == EZOStatus::Connected){
      if(device_list[i].device.get_name() == "conductivity")
      {
        get_ec_k_value();
        doc[device_list[i].device.get_name()]["k Value"] = k_val;
      }
      doc[device_list[i].device.get_name()]["Temperature Compensation"] = device_list[i].tempCompensation;
      singleDeviceStatePayload(device_list[i].device, doc);
    }
  }  
  if(DroneSensor_debug){serializeJsonPretty(doc, Serial);Serial.println("");}
  String output;
  serializeJson(doc, output);
  return output; 
}

bool DroneSensor::processCommand(StaticJsonDocument<DOC_SIZE>& _command){
  bool returnCode = true;
  for (int i = 0; i < device_list_len; i++ ){
  
    String command = _command[device_list[i].device.get_name()]["Command"];
    
    if(command != NULL and command != "null" ){
      String __command = _command[device_list[i].device.get_name()]["Command"];
      if (DroneSensor_debug) {Serial.println("Found command " + String(__command) + " for " + String(device_list[i].device.get_name()));}
      if(device_list[i]._status == EZOStatus::Connected){
        device_list[i].device.send_cmd(__command.c_str());
        select_delay(__command);
        char receive_buffer[32];
        if(device_list[i].device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
          if (DroneSensor_debug) {Serial.println("Success : Processing Command " + String(__command) +" for " + String(device_list[i].device.get_name()) );}
        }
        else
        {
          if (DroneSensor_debug) {Serial.println("Failed : Processing Command " + String(__command) +" for " + String(device_list[i].device.get_name()) );}        
          returnCode = false;
        }
      }
      else
      {
        if (DroneSensor_debug) {Serial.println("Failed : Processing Command " + String(__command) +" for " + String(device_list[i].device.get_name()) + " not connected");}
      }
    }
  } 
  return returnCode;
 }
   
bool DroneSensor::processConfig(StaticJsonDocument<DOC_SIZE>& _config){
  bool returnCode = true;
  if(_config["Fallback Temperature"] != NULL){
    float __fallbackTemperature = _config["Fallback Temperature"].as<float>();
    if (DroneSensor_debug) {Serial.println("Setting Fallback Temperature to " + String(__fallbackTemperature));}
    if(__fallbackTemperature > 0){
      setFallbackTemp(__fallbackTemperature);
    }
  }

  if(_config["Poll Delay"] != NULL){
    int __pollDelay = _config["Poll Delay"].as<int>();
    if (DroneSensor_debug) {Serial.println("Setting Poll Delay to " + String(__pollDelay));}
    if(__pollDelay > 100){
      this->pollDelay =__pollDelay;
    }
  }  

  if(_config["Parameters"] != NULL){
    if(_config["Parameters"] == "Off"){
      if (DroneSensor_debug) {Serial.println(F("Turning Parameters Off"));}
      turnParametersOff();
    }
    else
    {
      if (DroneSensor_debug) {Serial.println(F("Turning Parameters On"));}
      turnParametersOn();      
    }
  }
  
  if(_config["logData"] != NULL){
    if(_config["logData"] == "No"){
      if (DroneSensor_debug) {Serial.println(F("Turning Off logging data"));}
      this->loggingData = false;
    }else{
      if (DroneSensor_debug) {Serial.println(F("Turning on logging data"));}
      this->loggingData = true;
    }
  }
  return returnCode;
}


bool DroneSensor::headerPayload(StaticJsonDocument<DOC_SIZE>& _doc){
  _doc["deviceMAC"] = this->_deviceMAC;
  return true;
}
String DroneSensor::bootPayload(String _EpochTime){
  if (DroneSensor_debug) { Serial.println(F("DroneSensor::bootPayload()")); Serial.println(""); }
  StaticJsonDocument<DOC_SIZE> doc;
     
  doc["bootTime"] = _EpochTime;
  headerPayload(doc);
  doc["deviceName"] = String(this->_deviceID);
  doc["deviceIP"] = String(this->_deviceIP);

  if(DroneSensor_debug){serializeJsonPretty(doc, Serial);Serial.println("");}
    
  String payload;
  serializeJson(doc, payload);
  return payload;
}
void DroneSensor::test(){
   for (int i = 0; i < device_list_len; i++ ){
     if(device_list[i]._status == EZOStatus::Connected){
       String command = "O,?";
       char receive_buffer[32];
       String cmdReply;
       device_list[i].device.send_cmd(command.c_str());
       select_delay(command);
       if(device_list[i].device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
         cmdReply = String(receive_buffer);
       }
       else
       {
         cmdReply = "Read parameters failed";
       }
       command = "R";
       device_list[i].device.send_cmd(command.c_str());
       select_delay(command);       
       if(device_list[i].device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
         Serial.print(String(device_list[i].device.get_name()) + " parameters :- ");
         Serial.println(cmdReply);        //parse the reading into a float
         
         Serial.print(String(device_list[i].device.get_name()) + " returned :- ");
         Serial.println(String(receive_buffer));        //parse the reading into a float
       }
       else
       {
         Serial.print(String(device_list[i].device.get_name()) + " parameters :- ");
         Serial.println(cmdReply);        //parse the reading into a float
         
         Serial.print(String(device_list[i].device.get_name()) + " returned :- ");
         Serial.println(F("Read sensors failed"));        //parse the reading into a float
         
       }
       cmdReply="";
     }
     else
     {
        Serial.print(String(device_list[i].device.get_name()) + " is not connected");
     }
   } 
}
