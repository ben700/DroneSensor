#include "DroneSensor.h"

double showDecimals(const double &x, const int &numDecimals)
{
  int y = x;
  double z = x - y;
  double m = pow(10, numDecimals);
  double q = z * m;
  double r = round(q);

  return static_cast<double>(y) + (1.0 / m) * r;
}

DroneSensor::DroneSensor(String __deviceMAC, String __deviceIP, String __deviceID, bool _DroneSensor_debug = false)
{
  DroneSensor_debug = _DroneSensor_debug;
  this->_deviceMAC = __deviceMAC;
  this->_deviceIP = __deviceIP;
  this->_deviceID = __deviceID;
  Wire.begin(); 
  if (DroneSensor_debug) { Serial.println(F("DroneSensorAir::DroneSensor()"));}
  delay(long_delay);
  
  for (int i = 0; i < device_list_len; i++ )
  {
    byte address = device_list[i].device.get_address();
    Wire.beginTransmission(address);
    if (!Wire.endTransmission())
    {
      device_list[i]._status = EZOStatus::Connected;
      findDevice(device_list[i]);
      this->current_step = EZOReadingStep::REQUEST_TEMP;
      if (DroneSensor_debug) {
        Serial.print("EZO Circuit " + String(device_list[i].device.get_name()) + " found at address ");
        Serial.print(address);
        Serial.println(F("  !"));
        
      }
    }
    else
    {
      device_list[i]._status = EZOStatus::Unconnected;
      if (DroneSensor_debug) {
        Serial.print("EZO Circuit " + String(device_list[i].device.get_name()) + " NOT found at address ");
        Serial.print(address);
        Serial.println("  !");       
      }
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

void DroneSensor::turnParametersOn() {
  this->parametersOn = true;
  for (int i = 0; i < device_list_len; i++) {
  if (device_list[i].device.get_name() == CO2.get_name() and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("CO2 is "));}
      String command = "O,t,1";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == HUM.get_name() and device_list[i]._status == EZOStatus::Connected){
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
   if (device_list[i].device.get_name() == CO2.get_name() and device_list[i]._status == EZOStatus::Connected){
      if (DroneSensor_debug) {Serial.print(F("CO2 is "));}
      String command = "O,t,0";
      device_list[i].device.send_cmd(command.c_str());
      if (DroneSensor_debug) {Serial.println(F("reconfigured!"));}
    }else if (device_list[i].device.get_name() == HUM.get_name() and device_list[i]._status == EZOStatus::Connected){
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

void DroneSensor::sendReadCommand(StaticJsonDocument<DOC_SIZE>& _doc) {
  if (DroneSensor_debug) { Serial.println(F("DroneSensor::sendReadCommand()"));}

    for (int i = 0; i < device_list_len; i++ )
    {
      if(device_list[i]._status == EZOStatus::Connected){
        device_list[i].device.send_read_cmd();
        if (DroneSensor_debug) { Serial.println("DroneSensorAir::sendReadCommand() -> Sent read command to " + String(device_list[i].device.get_name())); }  
      }
    }
}


String DroneSensor::sensorPayload(String _EpochTime)
{
  if (DroneSensor_debug) { Serial.println(F("DroneSensorAir::sensorPayload()"));  }
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
              doc[device_list[i].device.get_name()][device_list[i]._parameterList[y]._payloadName] = showDecimals(atof(pReading), device_list[i]._parameterList[y]._precision);
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
    doc[Device.get_name()] = NotConnected;
  }else{
    cmdReply = String(receive_buffer);        //parse the reading into a float

    String type = cmdReply.substring(cmdReply.indexOf(",")+1, cmdReply.indexOf(",",4));
    String firm = cmdReply.substring(cmdReply.indexOf(",",4)+1);
  
 //   doc[Device.get_name()]["Name"] = Device.get_name();
    doc[Device.get_name()]["Firmware"] = firm;
  
    command = "CAL,?";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }
  
    String calibrationPoints = cmdReply.substring(cmdReply.indexOf("CAL,")+4);
   // doc[Device.get_name()]["Calibration Points"] = calibrationPoints;
    doc[Device.get_name()]["CP"] = calibrationPoints;
/*  
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
  
  */
  }
  return;
}


String DroneSensor::deviceStatePayload (StaticJsonDocument<DOC_SIZE> &doc){
 
 // doc["Fallback Temperature"] = this->_FallbackTemp;
  doc["Fallback Temp"] = this->_FallbackTemp;
  doc["Poll Delay"] = this->pollDelay;
  this->parametersOn ? doc["Parameters"] = "On" : doc["Parameters"] = "Off" ;
  
  for (int i = 0; i < device_list_len; i++ )
  {
    if(device_list[i]._status == EZOStatus::Connected){
     
    //  doc[device_list[i].device.get_name()]["Temperature Compensation"] = device_list[i].tempCompensation;
      doc[device_list[i].device.get_name()]["tempComp"] = device_list[i].tempCompensation;
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

