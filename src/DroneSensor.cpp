#include "DroneSensor.h"

DroneSensor::DroneSensor(String __deviceMAC, String __deviceIP, String __deviceID, bool _DroneSensor_debug = false)
{
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
  if (DroneSensor_debug) { Serial.println("DroneSensor::DroneSensor()");}
  delay(long_delay);
  
  for (int i = 0; i < device_list_len; i++ )
  {
    byte address = device_list[i].device.get_address();
    Wire.beginTransmission(address);
    if (!Wire.endTransmission())
    {
      device_list[i]._status = EZOStatus::Connected;
      this->current_step = EZOReadingStep::REQUEST_TEMP;
      if (DroneSensor_debug) {
        Serial.print("EZO Circuit " + String(device_list[i].device.get_name()) + " found at address ");
        Serial.print(address);
        Serial.println("  !");
        if(device_list[i].tempCompensation){
          device_list[i].device.send_cmd_with_num("T,", DroneSensor_FallbackTemp);
        }
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

void DroneSensor::list_devices() {
  for (int i = 0; i < device_list_len; i++) {        //go thorugh the list of boards
    print_device_info(device_list[i].device);                   //then print the boards info
    if(device_list[i]._status == EZOStatus::Unconnected)
    {
       Serial.print(" Unconnected");
    }else{
       Serial.print(" Connected");

      
    }
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

void DroneSensor::turnParametersOn() {
  for (int i = 0; i < device_list_len; i++) {
    if(device_list[i].device.get_name() == "conductivity" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("Conductivity is ");
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
      Serial.println("reconfigured!");
    }else if (device_list[i].device.get_name() == "DO" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("Dissolved Oxygen is ");
      String command = "O,mg,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,%,1";
      device_list[i].device.send_cmd(command.c_str());
      Serial.println("reconfigured!");
    }else if (device_list[i].device.get_name() == "CO2" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("CO2 is ");
      String command = "O,t,1";
      device_list[i].device.send_cmd(command.c_str());
      Serial.println("reconfigured!");
    }else if (device_list[i].device.get_name() == "HUM" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("Humitity is ");
      String command = "O,HUM,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,T,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,Dew,1";
      device_list[i].device.send_cmd(command.c_str());
      Serial.println("reconfigured!");
    }
  }
}
void DroneSensor::turnParametersOff() {
  for (int i = 0; i < device_list_len; i++) {
    if(device_list[i].device.get_name() == "conductivity" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("Conductivity is ");
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
      Serial.println("reconfigured!");
    }else if (device_list[i].device.get_name() == "DO" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("Dissolved Oxygen is ");
      String command = "O,mg,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,%,0";
      device_list[i].device.send_cmd(command.c_str());
      Serial.println("reconfigured!");
    }else if (device_list[i].device.get_name() == "CO2" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("CO2 is ");
      String command = "O,t,0";
      device_list[i].device.send_cmd(command.c_str());
      Serial.println("reconfigured!");
    }else if (device_list[i].device.get_name() == "HUM" and device_list[i]._status == EZOStatus::Connected){
      Serial.print("Humitity is ");
      String command = "O,HUM,1";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,T,0";
      device_list[i].device.send_cmd(command.c_str());
      select_delay(command);
      command = "O,Dew,0";
      device_list[i].device.send_cmd(command.c_str());
      Serial.println("reconfigured!");
    }
  }
}
void DroneSensor::receive_reading(Ezo_board &Device) {              // function to decode the reading after the read command was issued

  if (DroneSensor_debug) {Serial.print(Device.get_name()); Serial.print(": ");}
  Device.receive_read_cmd();              //get the response data and put it into the [Device].reading variable if successful

  if (DroneSensor_debug) {print_error_type(Device, String(Device.get_last_received_reading(), 2).c_str());}
}


void DroneSensor::sendReadCommand(StaticJsonDocument<DOC_SIZE>& _doc) {
  if (DroneSensor_debug) { Serial.println("DroneSensor::sendReadCommand()");}
  float temp = DroneSensor_FallbackTemp;
  if(device_list[0]._status == EZOStatus::Connected){
    device_list[0].device.send_read_cmd();
    delay(reading_delay);
    receive_reading(device_list[0].device);
    if (DroneSensor_debug) { print_error_type(device_list[0].device, "Reading Temp Success");} 
    if ((device_list[0].device.get_error() == Ezo_board::SUCCESS) && (device_list[0].device.get_last_received_reading() > -1000.0))
    {
      temp =device_list[0].device.get_last_received_reading();
      if (DroneSensor_debug) { Serial.print("DroneSensor::sendReadCommand RTD.get_last_received_reading() "); Serial.println(temp);}
    } 
  
    for (int i = 1; i < device_list_len; i++ )
    {
      if(device_list[i]._status == EZOStatus::Connected){
        device_list[i].device.send_read_with_temp_comp(temp);
        //device_list[i].device.send_read_cmd();
        if (DroneSensor_debug) { Serial.println("DroneSensor::sendReadCommand() -> Sent read command to " + String(device_list[i].device.get_name())); }  
      }
    }
  }else{
    for (int i = 1; i < device_list_len; i++ )
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
  if (DroneSensor_debug) { Serial.println("DroneSensor::sensorPayload()"); Serial.println(""); }
  StaticJsonDocument<DOC_SIZE> doc;
     
  doc["deviceTime"] = _EpochTime;

  headerPayload(doc);
  
  if (DroneSensor_debug) { Serial.println("Header ----------"); serializeJsonPretty(doc, Serial); Serial.println(""); }

  sendReadCommand(doc);
  delay(reading_delay);
  Serial.println("Send read now do read");
  for (int i = 1; i < device_list_len; i++ )
  {
    Serial.println("Processing " + String(device_list[i].device.get_name()));
    if(device_list[i]._status == EZOStatus::Connected){
      char receive_buffer[32];
      if(device_list[i].device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
        char * pReading;
        char delimiter[] = ",";
        pReading = strtok (receive_buffer,delimiter);
        for(int y=0; y < device_list[i]._countParameter; y++){
          if(pReading != NULL){
            if(device_list[i]._parameterList[y]._payloadName != NULL and device_list[i]._parameterList[y]._payloadName.length() >0){
              doc[device_list[i]._parameterList[y]._payloadName] = String(atof(pReading), device_list[i]._parameterList[y]._precision);
            }
          }else{
              Serial.println("Error: Null but expected to get " + device_list[i]._parameterList[y]._displayName + " for " + String(device_list[i].device.get_name()));
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
  
    doc["Name"] = Device.get_name();
    doc["Firmware"] = firm;
  
    command = "CAL,?";
    Device.send_cmd(command.c_str());
    select_delay(command);
    if(Device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
      cmdReply = String(receive_buffer);        //parse the reading into a float
    }
  
    String calibrationPoints = cmdReply.substring(cmdReply.indexOf("CAL,")+4);
  //  doc["Calibration Points"] = calibrationPoints;
    doc["CP"] = calibrationPoints;
  
    
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


String DroneSensor::deviceStatePayload (){
  String payload = "";
  for (int i = 0; i < device_list_len; i++ )
  {
    if(device_list[i]._status == EZOStatus::Connected){
      payload = payload + singleDeviceStatePayload(device_list[i].device);
    }
  }  
  return payload;
}

bool DroneSensor::processCommand(StaticJsonDocument<DOC_SIZE>& _command){
  bool returnCode = true;
  for (int i = 0; i < device_list_len; i++ ){
  
    String command = _command[device_list[i].device.get_name()]["Command"];
    
    if(command != NULL and command != "null" ){
      String __command = _command[device_list[i].device.get_name()]["Command"];
      Serial.println("Found command " + String(__command) + " for " + String(device_list[i].device.get_name()));
      if(device_list[i]._status == EZOStatus::Connected){
        device_list[i].device.send_cmd(__command.c_str());
        select_delay(__command);
        char receive_buffer[32];
        if(device_list[i].device.receive_cmd(receive_buffer, 32) == Ezo_board::SUCCESS){   //if the reading is successful
          Serial.println("Success : Processing Command " + String(__command) +" for " + String(device_list[i].device.get_name()) );
        }
        else
        {
          Serial.println("Failed : Processing Command " + String(__command) +" for " + String(device_list[i].device.get_name()) );          
          returnCode = false;
        }
      }
      else
      {
        Serial.println("Failed : Processing Command " + String(__command) +" for " + String(device_list[i].device.get_name()) + " not connected");
      }
    }
  } 
  return returnCode;
 }
   

String DroneSensor::calibrationCommands(String calibrationCommandError){
    StaticJsonDocument<DOC_SIZE> doc;
     
    if(calibrationCommandError.length() > 0)  doc["Error"] = calibrationCommandError;
    String _deviceName; 
    int i =0;
    if(device_list[i]._status == EZOStatus::Connected){
      _deviceName = device_list[i++].device.get_name(); //get name then inc i.e. this will return zero then set i =1 for next command
      doc["Calibration Commands"][_deviceName]["deviceID"] = 102;
      doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
      doc["Calibration Commands"][_deviceName]["Set to t"] = "Cal,t";
    }
    
    if(device_list[i]._status == EZOStatus::Connected){
      _deviceName = device_list[i++].device.get_name();    
      doc["Calibration Commands"][_deviceName]["deviceID"] = 100;
      doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
      doc["Calibration Commands"][_deviceName]["Atmospheric"] = "Cal,dry";
      doc["Calibration Commands"][_deviceName]["Single Point"] = "Cal,n";
      doc["Calibration Commands"][_deviceName]["Low"] = "Cal,low,n";
      doc["Calibration Commands"][_deviceName]["High"] = "Cal,high,n";
      doc["Calibration Commands"][_deviceName]["probe Type"] = "K,n";
    }

    if(device_list[i]._status == EZOStatus::Connected){
      _deviceName = device_list[i++].device.get_name();    
      doc["Calibration Commands"][_deviceName]["deviceID"] = 99;
      doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
      doc["Calibration Commands"][_deviceName]["Mid"] = "Cal,mid,7";
      doc["Calibration Commands"][_deviceName]["Low"] = "Cal,low,4";
      doc["Calibration Commands"][_deviceName]["High"] = "Cal,high,10";
    }
    
    if(device_list[i]._status == EZOStatus::Connected){
      _deviceName = device_list[i++].device.get_name();    
      doc["Calibration Commands"][_deviceName]["deviceID"] = 97;
      doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
      doc["Calibration Commands"][_deviceName]["Atmospheric"] = "Cal";
      doc["Calibration Commands"][_deviceName]["Solution"] = "Cal,0";
    }
  
    if(device_list[i]._status == EZOStatus::Connected){
      _deviceName = device_list[i].device.get_name();
      doc["Calibration Commands"][_deviceName]["deviceID"] = 98;
      doc["Calibration Commands"][_deviceName]["Clear"] = "Cal,clear";
      doc["Calibration Commands"][_deviceName]["Set to n"] = "Cal,n";
    }

     
    if(DroneSensor_debug){serializeJsonPretty(doc, Serial);Serial.println("");}
    
  
    String payload;
    serializeJson(doc, payload);
  
    return payload;
}
bool DroneSensor::headerPayload(StaticJsonDocument<DOC_SIZE>& _doc){
  _doc["deviceMAC"] = this->_deviceMAC;
  return true;
}
String DroneSensor::bootPayload(String _EpochTime){
  if (DroneSensor_debug) { Serial.println("DroneSensor::bootPayload()"); Serial.println(""); }
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
         Serial.println("Read sensors failed");        //parse the reading into a float
         
       }
       cmdReply="";
     }
     else
     {
        Serial.print(String(device_list[i].device.get_name()) + " is not connected");
     }
   } 
}
