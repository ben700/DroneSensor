#include <Ezo_i2c.h> //include the EZO I2C library from https://github.com/Atlas-Scientific/Ezo_I2c_lib
#include <Wire.h>    //include arduinos i2c library
#include <ESP8266WiFi.h>                                         //include esp8266 wifi library 

WiFiClient  client;                                              //declare that this device connects to a Wi-Fi network,create a connection to a specified internet IP address

//----------------Fill in your Wi-Fi / ThingSpeak Credentials-------
const String ssid = "HartleyAvenue";                                 //The name of the Wi-Fi network you are connecting to
const String pass = "33HartleyAvenue";                             //Your WiFi network password


Ezo_board PH = Ezo_board(99, "PH");       //create a PH circuit object, who's address is 99 and name is "PH"
Ezo_board EC = Ezo_board(100, "EC");      //create an EC circuit object who's address is 100 and name is "EC"
Ezo_board RTD = Ezo_board(102, "RTD");    //create an RTD circuit object who's address is 102 and name is "RTD"
Ezo_board DO = Ezo_board(97, "DO");    //create a DO circuit object who's address is 97 and name is "DO"

//array of ezo boards, add any new boards in here for the commands to work with them
Ezo_board device_list[] = {
  PH,
  EC,
  RTD,
  DO
};

//enable pins for each circuit
const int EN_PH = 14;
const int EN_EC = 12;
const int EN_RTD = 15;
const int EN_AUX = 13;

Ezo_board* default_board = &device_list[0]; //used to store the board were talking to

//gets the length of the array automatically so we dont have to change the number every time we add new boards
const uint8_t device_list_len = sizeof(device_list) / sizeof(device_list[0]);

enum reading_step {REQUEST_TEMP, READ_TEMP_AND_COMPENSATE, REQUEST_DEVICES, READ_RESPONSE };          //the readings are taken in 3 steps
//step 1 tell the temp sensor to take a reading
//step 2 consume the temp reading and send it to the devices
//step 4 tell the devices to take a reading based on the temp reading we just received
//step 3 consume the devices readings

enum reading_step current_step = REQUEST_TEMP;                                    //the current step keeps track of where we are. lets set it to REQUEST_TEMP (step 1) on startup


int return_code = 0;                      //holds the return code sent back from thingSpeak after we upload the data

uint32_t next_step_time = 0;              //holds the next time we receive a response, in milliseconds
const unsigned long reading_delay = 1000;  //how long we wait to receive a response, in milliseconds
unsigned int poll_delay = 0;              //how long we wait between reading, in milliseconds

const unsigned long thingspeak_delay = 15000;                                     //how long we wait to send something to thingspeak

const unsigned long short_delay = 300;              //how long we wait for most commands and queries
const unsigned long long_delay = 1200;              //how long we wait for commands like cal and R (see datasheets for which commands have longer wait times)

float k_val = 0;                                    //holds the k value of the ec circuit

void setup() {

  pinMode(EN_PH, OUTPUT);                                                         //set enable pins as outputs
  pinMode(EN_EC, OUTPUT);
  pinMode(EN_RTD, OUTPUT);
  pinMode(EN_AUX, OUTPUT);
  digitalWrite(EN_PH, LOW);                                                       //set enable pins to enable the circuits
  digitalWrite(EN_EC, LOW);
  digitalWrite(EN_RTD, HIGH);
  digitalWrite(EN_AUX, LOW);

  Wire.begin();                           //start the I2C
  Serial.begin(9600);                     //start the serial communication to the computer
  
}

void loop() {
 
  
    switch (current_step) {                                                         //selects what to do based on what reading_step we are in
      //------------------------------------------------------------------

      case REQUEST_TEMP:                                                            //when we are in the first step
        if (millis() >= next_step_time) {                                           //check to see if enough time has past, if it has
          RTD.send_read_cmd();
          next_step_time = millis() + reading_delay; //set when the response will arrive
          current_step = READ_TEMP_AND_COMPENSATE;       //switch to the receiving phase
        }
        break;

      //------------------------------------------------------------------
      case READ_TEMP_AND_COMPENSATE:
        if (millis() >= next_step_time) {

          receive_reading(RTD);             //get the reading from the RTD circuit

          if ((RTD.get_error() == Ezo_board::SUCCESS) && (RTD.get_last_received_reading() > -1000.0)) { //if the temperature reading has been received and it is valid
            
            PH.send_cmd_with_num("T,", RTD.get_last_received_reading());
            EC.send_cmd_with_num("T,", RTD.get_last_received_reading());
            DO.send_cmd_with_num("T,", RTD.get_last_received_reading());
          } else {                                                                                      //if the temperature reading is invalid
            PH.send_cmd_with_num("T,", 25.0);
            EC.send_cmd_with_num("T,", 25.0);                                                          //send default temp = 25 deg C to EC sensor
            DO.send_cmd_with_num("T,", 20.0);
          }

          Serial.print(" ");
          next_step_time = millis() + short_delay; //set when the response will arrive
          current_step = REQUEST_DEVICES;        //switch to the receiving phase
        }
        break;

      //------------------------------------------------------------------
      case REQUEST_DEVICES:                      //if were in the phase where we ask for a reading
        if (millis() >= next_step_time) {
          //send a read command. we use this command instead of PH.send_cmd("R");
          //to let the library know to parse the reading
          PH.send_read_cmd();
          EC.send_read_cmd();
          DO.send_read_cmd();

          next_step_time = millis() + reading_delay; //set when the response will arrive
          current_step = READ_RESPONSE;              //switch to the next step
        }
        break;

      //------------------------------------------------------------------
      case READ_RESPONSE:                             //if were in the receiving phase
        if (millis() >= next_step_time) {  //and its time to get the response

          receive_reading(PH);             //get the reading from the PH circuit
          Serial.print("  ");
          receive_reading(EC);             //get the reading from the EC circuit
          Serial.print("  ");
          receive_reading(DO);             //get the reading from the DO circuit
   

          Serial.println();

  
          next_step_time =  millis() + poll_delay;
          current_step = REQUEST_TEMP;                                                          //switch back to asking for readings
        }
        break;
    }
}



// determines how long we wait depending on the command
void select_delay(String &str) {
  if (str.indexOf("CAL") != -1 || str.indexOf("R") != -1) {
    delay(long_delay);
  } else {
    delay(short_delay);
  }
}

// prints the boards name and I2C address
void print_device_info(Ezo_board &Device) {
  Serial.print(Device.get_name());
  Serial.print(" ");
  Serial.print(Device.get_address());
}

void print_device_response(Ezo_board &Device) {
  char receive_buffer[32];                  //buffer used to hold each boards response
  Device.receive_cmd(receive_buffer, 32);   //put the response into the buffer

  print_error_type(Device, " - ");          //print if our response is an error or not
  print_device_info(Device);                //print our boards name and address
  Serial.print(": ");
  Serial.println(receive_buffer);           //print the boards response
}

void list_devices() {
  for (uint8_t i = 0; i < device_list_len; i++) {        //go thorugh the list of boards
    if (default_board == &device_list[i]) {              //if its our default board
      Serial.print("--> ");                             //print the pointer arrow
    } else {                                            //otherwise
      Serial.print(" - ");                              //print a normal dash
    }
    print_device_info(device_list[i]);                   //then print the boards info
    Serial.println("");
  }
}



// used for printing either a success_string message if a command was successful or the error type if it wasnt
void print_error_type(Ezo_board &Device, const char* success_string) {
  switch (Device.get_error()) {             //switch case based on what the response code is.
    case Ezo_board::SUCCESS:
      Serial.print(success_string);   //the command was successful, print the success string
      break;

    case Ezo_board::FAIL:
      Serial.print("Failed ");        //means the command has failed.
      break;

    case Ezo_board::NOT_READY:
      Serial.print("Pending ");       //the command has not yet been finished calculating.
      break;

    case Ezo_board::NO_DATA:
      Serial.print("No Data ");       //the sensor has no data to send.
      break;
  }
}

void get_ec_k_value(){                                    //function to query the value of the ec circuit
  char rx_buf[10];                                        //buffer to hold the string we receive from the circuit
  EC.send_cmd("k,?");                                     //query the k value
  delay(300);
  if(EC.receive_cmd(rx_buf, 10) == Ezo_board::SUCCESS){   //if the reading is successful
    k_val = String(rx_buf).substring(3).toFloat();        //parse the reading into a float
  }
}

void receive_reading(Ezo_board &Device) {              // function to decode the reading after the read command was issued

  Serial.print(Device.get_name()); Serial.print(": "); // print the name of the circuit getting the reading

  Device.receive_read_cmd();              //get the response data and put it into the [Device].reading variable if successful

  print_error_type(Device, String(Device.get_last_received_reading(), 2).c_str());  //print either the reading or an error message
}

void print_help() {
  get_ec_k_value();
  Serial.println(F("Atlas Scientific I2C hydroponics kit                                       "));
}
