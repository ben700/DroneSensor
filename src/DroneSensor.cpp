#include "DroneSensor.h"

DroneSensor::DroneSensor(String __deviceMAC, String __deviceIP)
{
  this->_deviceMAC = __deviceMAC;
  this->_deviceIP = __deviceIP;
  Wire.begin(); 
  pinMode(EN_PH, OUTPUT);                                                         //set enable pins as outputs
  pinMode(EN_EC, OUTPUT);
  pinMode(EN_RTD, OUTPUT);
  pinMode(EN_AUX, OUTPUT);
  digitalWrite(EN_PH, LOW);                                                       //set enable pins to enable the circuits
  digitalWrite(EN_EC, LOW);
  digitalWrite(EN_RTD, HIGH);
  digitalWrite(EN_AUX, LOW); 
  if (DroneSensor_debug) { Serial.println("DroneSensor::DroneSensor()");}
  delay(long_delay);
  
  for (int i = 0; i <= device_list_len; i++ )
  {
    address = device_list[i]->device.get_address();
    Wire.beginTransmission(address);
    if (!Wire.endTransmission())
    {
      device_list[i]->_status = EZOStatus::Connected;
      if (DroneSensor_debug) {
        Serial.print("I2C " + device_list[i]->device.get_name() + " found at address ");
        Serial.print(address);
        Serial.println("  !"); 
      }
    }
    else
    {
      device_list[i]->_status = EZOStatus::Unconnected;
      if (DroneSensor_debug) {
        Serial.print("I2C " + device_list[i]->device.get_name() + " NOT found at address ");
        Serial.print(address);
        Serial.println("  !");       
    }
  }

  PH.send_cmd_with_num("T,", DroneSensor_FallbackTemp);
  EC.send_cmd_with_num("T,", DroneSensor_FallbackTemp);
  DO.send_cmd_with_num("T,", DroneSensor_FallbackTemp);
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
    return "Powered Off";
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
void DroneSensor::list_devices() {
  for (uint8_t i = 0; i < device_list_len; i++) {        //go thorugh the list of boards
    if (default_board == &device_list[i]->device) {              //if its our default board
      Serial.print("--> ");                             //print the pointer arrow
    } else {                                            //otherwise
      Serial.print(" - ");                              //print a normal dash
    }
    print_device_info(device_list[i]->device);                   //then print the boards info
    Serial.print(device_list[i]->_status);
    Serial.println("");
  }
}
void DroneSensor::get_ec_k_value(){                                    //function to query the value of the ec circuit
  char rx_buf[10];                                        //buffer to hold the string we receive from the circuit
  EC.send_cmd("k,?");                                     //query the k value
  delay(300);
  if(EC.receive_cmd(rx_buf, 10) == Ezo_board::SUCCESS){   //if the reading is successful
    k_val = String(rx_buf).substring(3).toFloat();        //parse the reading into a float
  }
}


void DroneSensor::print_help() {
  get_ec_k_value();
  Serial.println(F("Atlas Scientific I2C hydroponics kit                                       "));
  if(k_val > 9){
     Serial.println(F("For K10 probes, these are the recommended calibration values:            "));
  }
  else if(k_val > .9){
     Serial.println(F("For K1 probes, these are the recommended calibration values:             "));
  }
  else if(k_val > .09){
     Serial.println(F("For K0.1 probes, these are the recommended calibration values:           "));
  }
  
}


// prints the boards name and I2C address
void DroneSensor::print_device_info(Ezo_board &Device) {
  Serial.print(Device.get_name());
  Serial.print(" ");
  Serial.print(Device.get_address());
}

void DroneSensor::print_device_response(Ezo_board &Device) {
  char receive_buffer[32];                  //buffer used to hold each boards response
  Device.receive_cmd(receive_buffer, 32);   //put the response into the buffer

  print_error_type(Device, " - ");          //print if our response is an error or not
  print_device_info(Device);                //print our boards name and address
  Serial.print(": ");
  Serial.println(receive_buffer);           //print the boards response
}

void DroneSensor::receive_reading(Ezo_board &Device) {              // function to decode the reading after the read command was issued

  if (DroneSensor_debug) {Serial.print(Device.get_name()); Serial.print(": ");}
  Device.receive_read_cmd();              //get the response data and put it into the [Device].reading variable if successful

  if (DroneSensor_debug) {print_error_type(Device, String(Device.get_last_received_reading(), 2).c_str());}
}

String DroneSensor::readTemp() {              // function to decode the reading after the read command was issued
  RTD.send_read_cmd();
  receive_reading(RTD);
  print_error_type(RTD, String(RTD.get_last_received_reading(), 1).c_str());  //print either the reading or an error message
  return return_error_type(RTD, String(RTD.get_last_received_reading(), 1).c_str());
}


String DroneSensor::readEC() {              // function to decode the reading after the read command was issued
  (RTD.get_error() == Ezo_board::SUCCESS) ? EC.send_read_with_temp_comp(RTD.get_last_received_reading()) : EC.send_read_with_temp_comp(19.5);
  receive_reading(EC);
  return return_error_type(EC, String(EC.get_last_received_reading()).c_str());
}

String DroneSensor::readPH() {              // function to decode the reading after the read command was issued
  (RTD.get_error() == Ezo_board::SUCCESS) ? PH.send_read_with_temp_comp(RTD.get_last_received_reading()) : PH.send_read_with_temp_comp(19.5);
  receive_reading(PH);
  return return_error_type(PH, String(PH.get_last_received_reading()).c_str());
}

String DroneSensor::readDO() {              // function to decode the reading after the read command was issued
  (RTD.get_error() == Ezo_board::SUCCESS) ? DO.send_read_with_temp_comp(RTD.get_last_received_reading()) : DO.send_read_with_temp_comp(19.5);
  receive_reading(DO);
  return return_error_type(DO, String(DO.get_last_received_reading()).c_str());
}

void DroneSensor::sendReadCommand(StaticJsonDocument<DOC_SIZE>& _doc) {
  if (DroneSensor_debug) { Serial.println("DroneSensor::sendReadCommand()");}
  float temp = DroneSensor_FallbackTemp;
  RTD.send_read_cmd();
  delay(reading_delay);
  receive_reading(RTD);
  if (DroneSensor_debug) { print_error_type(RTD, "Reading Temp Success");} 
  if ((RTD.get_error() == Ezo_board::SUCCESS) && (RTD.get_last_received_reading() > -1000.0))
  {
    temp =RTD.get_last_received_reading();
    if (DroneSensor_debug) { Serial.print("DroneSensor::sendReadCommand RTD.get_last_received_reading() "); Serial.println(temp);}
    _doc[RTD.get_name()] = String(temp, 1);    
  } 
  
  _doc[RTD.get_name()] = temp;
  _doc[RTD.get_name()]["return_code"] = return_error_type(RTD, "Success");
  PH.send_read_with_temp_comp(temp);
  EC.send_read_with_temp_comp(temp);
  DO.send_read_with_temp_comp(temp);
}

String DroneSensor::sensorPayload(String _EpochTime)
{
  if (DroneSensor_debug) { Serial.println("DroneSensor::sensorPayload()"); Serial.println(""); }
  StaticJsonDocument<DOC_SIZE> doc;
     
  doc["deviceTime"] = _EpochTime;

  headerPayload(doc);

  
  if (DroneSensor_debug) { Serial.println("Header ----------"); serializeJsonPretty(doc, Serial); Serial.println(""); }

 
  sendReadCommand(doc);
  delay(reading_delay);
  receive_reading(PH);
  doc[PH.get_name()] = return_error_type(PH, String(PH.get_last_received_reading(), 2));
 // (PH.get_error() == Ezo_board::SUCCESS) ? doc[PH.get_name()] = String(PH.get_last_received_reading(), 2) : doc[PH.get_name()] = NotConnected;
  receive_reading(EC);
  (EC.get_error() == Ezo_board::SUCCESS) ? doc[EC.get_name()] = String(EC.get_last_received_reading(), 2) : doc[EC.get_name()] = NotConnected;
  receive_reading(DO);
  (DO.get_error() == Ezo_board::SUCCESS) ? doc[DO.get_name()] = String(DO.get_last_received_reading(), 2) : doc[DO.get_name()] = NotConnected;
  
  DroneSensor_debug ? serializeJsonPretty(doc, Serial): Serial.println("");
  String output;
  serializeJson(doc, output);
  return output; 
}

String DroneSensor::singleDeviceStatePayload (Ezo_board &Device){

  String command = "I";
  String cmdReply;
  StaticJsonDocument<DOC_SIZE> doc;
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
  
    doc["DeviceName"] = Device.get_name();
    doc["Firmware"] = firm;
  
    command = "CAL,?";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }
  
    String calibrationPoints = cmdReply.substring(cmdReply.indexOf("CAL,")+4);
    doc["Calibration Points"] = calibrationPoints;
  
    
    command = "Status";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }

    String reasonForRestart = cmdReply.substring(cmdReply.indexOf(",")+1,cmdReply.indexOf(",", cmdReply.indexOf(",")+1) );
    String VoltageatVcc = cmdReply.substring(cmdReply.indexOf(",", cmdReply.indexOf(",")+1)+1); 
    doc["Restart"] = lookupRestartCodes(reasonForRestart);
    doc["Vcc"] = VoltageatVcc;

    
    command = "L,?";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }

    String LED = cmdReply.substring(cmdReply.indexOf("L,")+2);
    doc["LED"] = lookupLedStatus(LED);
  }
  
  if(DroneSensor_debug){serializeJsonPretty(doc, Serial);Serial.println("");}
  
  String output;
  serializeJson(doc, output);
  return output; 
}

String DroneSensor::tempStatePayload (){
  return singleDeviceStatePayload(RTD);
}
String DroneSensor::phStatePayload (){
  return singleDeviceStatePayload(PH);
}
String DroneSensor::ecStatePayload (){
  return singleDeviceStatePayload(EC);
}

String DroneSensor::deviceStatePayload (){
  String payload = payload + singleDeviceStatePayload(RTD);
  payload = payload + singleDeviceStatePayload(PH);
  payload = payload + singleDeviceStatePayload(EC);
  payload = payload + singleDeviceStatePayload(DO);
  return payload;
}

String DroneSensor::calibrationCommands(String calibrationCommandError){
    StaticJsonDocument<DOC_SIZE> doc;
     
    if(calibrationCommandError.length() > 0)  doc["Error"] = calibrationCommandError;
    String _deviceName = RTD.get_name();
    doc["Calibration Commands"][_deviceName]["deviceID"] = 102;
    doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
    doc["Calibration Commands"][_deviceName]["Set to t"] = "Cal,t";
    
    _deviceName = DO.get_name();    
    doc["Calibration Commands"][_deviceName]["deviceID"] = 97;
    doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
    doc["Calibration Commands"][_deviceName]["Atmospheric"] = "Cal";
    doc["Calibration Commands"][_deviceName]["Solution"] = "Cal,0";
  
    _deviceName = ORP.get_name();    
    doc["Calibration Commands"][_deviceName]["deviceID"] = 98;
    doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
    doc["Calibration Commands"][_deviceName]["Set to n"] = "Cal,n";


    _deviceName = EC.get_name();    
    doc["Calibration Commands"][_deviceName]["deviceID"] = 100;
    doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
    doc["Calibration Commands"][_deviceName]["Atmospheric"] = "Cal,dry";
    doc["Calibration Commands"][_deviceName]["Single Point"] = "Cal,n";
    doc["Calibration Commands"][_deviceName]["Low"] = "Cal,low,n";
    doc["Calibration Commands"][_deviceName]["High"] = "Cal,high,n";
    doc["Calibration Commands"][_deviceName]["probe Type"] = "K,n";
   

    _deviceName = PH.get_name();    
    doc["Calibration Commands"][_deviceName]["deviceID"] = 99;
    doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
    doc["Calibration Commands"][_deviceName]["Mid"] = "Cal,mid,7";
    doc["Calibration Commands"][_deviceName]["Low"] = "Cal,low,4";
    doc["Calibration Commands"][_deviceName]["High"] = "Cal,high,10";
    
    if(DroneSensor_debug){serializeJsonPretty(doc, Serial);}
    Serial.println("");
  
    String payload;
    serializeJson(doc, payload);
  
    return payload;
}
bool DroneSensor::headerPayload(StaticJsonDocument<DOC_SIZE>& _doc){
  _doc["deviceMAC"] = this->_deviceMAC;
  return true;
}
