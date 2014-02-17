/*
 *
 * Description :
 *
 * GPRS Weather station configured to collect temperature, relative humidity, pressure, rain and wind data avery 5 mn, store 
 * the corresponding values and other monitoring informations in the EEPROM and post them (as a file) to a web server every 
 * 5 or 15 minutes, depending on the battery's (dis)charge state.
 *
 * The Atmega is put in "sleep" mode between these tasks, and the GPS and GPRS modules are also switched off during this interlude 
 * in order to reduce the power consumption of the station. 
 *
 * The global position of the station is updated every 6 hours : the acquisition and upload tasks can be easily rescheduled 
 * in the scheduleNextTaskAndSleep() function.
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
 * Version : 0.8.0
 *
 * Creation date : 2014/02/14
 * 
 */
 

#include "EEPROM.h"

#include "SoftwareSerial.h"

#include "LowPower.h"

#include "Wire.h"

#include "Rtc_Pcf8563.h"

#include "Adafruit_BMP085.h"

#include "SHT1x.h"

#include "GPRSbee.h"

#include "Ultimate_GPS.h"

#include "MStore_24LC1025.h"

#include "WindRainMeter_DavisSensors.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// useful macros definitions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GET_STRING_LENGTH(s) (sizeof(s) / sizeof(s[0]))




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// General configuration parameters
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Firmware version

#define FIRMWARE_VERSION "0.8.0"   



// GPRS connection parameters

#define GPRS_NETWORK_APN "your_apn"                                       

#define GPRS_USERNAME "your_username"
#define GPRS_PASSWORD "your_password"                                



// Server connection parameters

#define SERVER_NAME "my_server.com"                      
#define SERVER_PORT "80"                                      

#define SERVER_GET_TIME_URL "/time.php"                       

#define SERVER_POST_REPORTS_URL "/post.php"                 
#define SERVER_POST_REPORTS_FILE_FORM_FIELD_NAME "file"                                  



// GPS acquisition parameters

#define GPS_FIRST_ACQUISITION_TIMEOUT_IN_SECONDS 600
#define GPS_FIRST_ACQUISITION_HDOP_LIMIT 10

#define GPS_ACQUISITION_TIMEOUT_IN_SECONDS 160
#define GPS_ACQUISITION_HDOP_LIMIT 10



// power saving / reports upload trigger values

#define POWER_SAVING_OPTION_ACTIVATED 1

#define POWER_SAVING_REPORTS_UPLOAD_BATTERY_VOLTAGE_TRIGGER_VALUE 3.9         

#define POWER_SAVING_REPORTS_UPLOAD_NUM_STORED_MESSAGES_TRIGGER_VALUE 3




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Hardware configuration parameters
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Pins definition

#define DEBUG_TX_PIN 3
#define DEBUG_RX_PIN 255

#define GPS_POWER_PIN 4

#define MODEM_POWER_PIN 5
#define MODEM_TX_PIN 6
#define MODEM_RX_PIN 7
#define MODEM_STATUS_PIN 8

#define DAVIS_RH_T_SENSOR_CLOCK_PIN 9     
#define DAVIS_RH_T_SENSOR_DATA_PIN 10           
     
#define DEVICE_BATTERY_VOLTAGE_PIN A3



// Station ID retrieval configuration (reminder : station ID is stored in the ATMEGA328's internal EEPROM, in it's 8 first bytes)

#define INTERNAL_EEPROM_STATION_ID_ADDRESS_START 0

#define STATION_ID_BUFFER_LENGTH 9



// EEPROM STORE I2C device adress

#define EEPROM_STORE_ADDRESS 0x53       // A0 and A1 pins of the 24LC1025 chip are tied to VCC



// Raind and wind sensors configuration

#define RAIN_GAUGE_SENSOR_CONNECTED_AND_ACTIVATED true

#define ANEMOMETER_SENSOR_CONNECTED_AND_ACTIVATED true



// ADC calibration data

#define ADC_REF_VOLTAGE 3.35




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// More configuration parameters
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// report buffer length

#define REPORT_BUFFER_LENGTH 125



// T, RH, P and V "undefined values" definition

#define EXTERNAL_TEMPERATURE_UNDEFINED_VALUE -100
#define EXTERNAL_RELATIVE_HUMIDITY_UNDEFINED_VALUE -10
#define PRESSURE_UNDEFINED_VALUE 0

#define BATTERY_VOLTAGE_UNDEFINED_VALUE -1
#define DEVICE_TEMPERATURE_UNDEFINED_VALUE -100



// tasks identifiers

#define TASK_NONE 0
#define TASK_READ_SENSORS_AND_STORE_REPORT 1
#define TASK_UPLOAD_STORED_REPORTS 2
#define TASK_GPS_ACQUIRE_POSITION_UPDATE_RTC 3




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Global variables definitions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



char stationID[STATION_ID_BUFFER_LENGTH] = "";

  
#define FIRMWARE_VERSION_STRING_LENGTH GET_STRING_LENGTH(FIRMWARE_VERSION)
  
char firmwareVersion[FIRMWARE_VERSION_STRING_LENGTH];


#define GPRS_NETWORK_APN_STRING_LENGTH GET_STRING_LENGTH(GPRS_NETWORK_APN)
#define GPRS_USERNAME_STRING_LENGTH GET_STRING_LENGTH(GPRS_USERNAME)
#define GPRS_PASSWORD_STRING_LENGTH GET_STRING_LENGTH(GPRS_PASSWORD)
  
char gprsNetworkAPN[GPRS_NETWORK_APN_STRING_LENGTH];
char gprsUsername[GPRS_USERNAME_STRING_LENGTH];
char gprsPassword[GPRS_PASSWORD_STRING_LENGTH];
  

#define SERVER_NAME_STRING_LENGTH GET_STRING_LENGTH(SERVER_NAME)
#define SERVER_PORT_STRING_LENGTH GET_STRING_LENGTH(SERVER_PORT)

char serverName[SERVER_NAME_STRING_LENGTH];
char serverPort[SERVER_PORT_STRING_LENGTH];


#define SERVER_GET_TIME_URL_STRING_LENGTH GET_STRING_LENGTH(SERVER_GET_TIME_URL)

char serverGetTimeURL[SERVER_GET_TIME_URL_STRING_LENGTH];


#define SERVER_POST_REPORTS_URL_STRING_LENGTH GET_STRING_LENGTH(SERVER_POST_REPORTS_URL)
#define SERVER_POST_REPORTS_FILE_FORM_FIELD_NAME_STRING_LENGTH GET_STRING_LENGTH(SERVER_POST_REPORTS_FILE_FORM_FIELD_NAME)
  
char serverPostReportsURL[SERVER_POST_REPORTS_URL_STRING_LENGTH];
char serverPostReportsFileFormFieldName[SERVER_POST_REPORTS_FILE_FORM_FIELD_NAME_STRING_LENGTH]; 



SoftwareSerial softSerialDebug(DEBUG_RX_PIN, DEBUG_TX_PIN); 


GPRSbee modem(MODEM_POWER_PIN, MODEM_STATUS_PIN, MODEM_RX_PIN, MODEM_TX_PIN, &softSerialDebug);


UltimateGPS gps(&Serial, GPS_POWER_PIN, &softSerialDebug);


MStore_24LC1025 mStore(EEPROM_STORE_ADDRESS);


Rtc_Pcf8563 rtc;


SHT1x temperatureHumiditySensor(DAVIS_RH_T_SENSOR_DATA_PIN, DAVIS_RH_T_SENSOR_CLOCK_PIN);


Adafruit_BMP085 pressureSensor;


WindRainMeter windRainMeter;



boolean rtcDateTimeSet = false;



byte nextTaskID = TASK_NONE;

unsigned long nextTaskTimestamp = 0;



byte reportCounter = 0;

unsigned long previousReportTimeStamp = 0;




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// setup() and loop() functions definition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup()  {
  
  
  // strings retrieval from flash
  
  retrieveStationIdFromInternalEEPROM(stationID);
  
  loadStringFromFlash(firmwareVersion, F(FIRMWARE_VERSION));
  
  loadStringFromFlash(gprsNetworkAPN, F(GPRS_NETWORK_APN));
  loadStringFromFlash(gprsUsername, F(GPRS_USERNAME));
  loadStringFromFlash(gprsPassword, F(GPRS_PASSWORD));
  
  loadStringFromFlash(serverName, F(SERVER_NAME));
  loadStringFromFlash(serverPort, F(SERVER_PORT));
  
  loadStringFromFlash(serverGetTimeURL, F(SERVER_GET_TIME_URL));
  
  loadStringFromFlash(serverPostReportsURL, F(SERVER_POST_REPORTS_URL));
  loadStringFromFlash(serverPostReportsFileFormFieldName, F(SERVER_POST_REPORTS_FILE_FORM_FIELD_NAME));
  
  
  softSerialDebug.begin(9600);
  
  gps.init(9600);
  
  modem.init(4800);

  Wire.begin();
  
  pressureSensor.begin(); 

  mStore.init();
  
  softSerialDebug.println(F("go !"));

  delay(5000);     // required to obtain a reliable response from the following modem.isOn() call 
 
  if(modem.isOn()) modem.powerOff();
  
  delay(1000);



  // date and time acquisition from server
  
  while(1) {
    
    rtcDateTimeSet = modemPowerOn_httpGetTimeInfoFromServer_updateRTC_modemPowerOff();
    
    if(rtcDateTimeSet) break;
    else sleepSeconds(30);
    
  }
  
  
  
  boolean taskSuccess;

  taskSuccess = readSensorsAndStoreReport();
    
  taskSuccess = modemPowerOn_httpPostStoredReports_modemPowerOff(1024);
    
  
  
  // date, time and first position acquisition from GPS
  
  taskSuccess = gpsPowerOn_acquireCurrentPosition_updateRTC_gpsPowerOff(GPS_FIRST_ACQUISITION_HDOP_LIMIT, GPS_FIRST_ACQUISITION_TIMEOUT_IN_SECONDS);
  
  taskSuccess = readSensorsAndStoreReport();
    
  taskSuccess = modemPowerOn_httpPostStoredReports_modemPowerOff(1024);
 
}



void loop() {
  
  scheduleNextTaskAndSleep();
  
  executeScheduledTask();
  
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Flash strings extraction functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void loadStringFromFlash(char *stringBuffer, const __FlashStringHelper *FString) {
  
  prog_char *FStringPtr = (prog_char*) FString;
  
  byte FStringPtrLength = 0;

  while(pgm_read_byte_near( FStringPtr + FStringPtrLength ) != '\0' ) FStringPtrLength++;
    
  memcpy_P(stringBuffer, FStringPtr, (FStringPtrLength + 1));
  
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Station ID retrieval (from internal EEPROM) functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void retrieveStationIdFromInternalEEPROM(char stationID[STATION_ID_BUFFER_LENGTH]) {
    
  for(byte i = 0 ; i < (STATION_ID_BUFFER_LENGTH - 1) ; i++) {
    
   stationID[i] = EEPROM.read(INTERNAL_EEPROM_STATION_ID_ADDRESS_START + i);
   
  }
  
  stationID[(STATION_ID_BUFFER_LENGTH - 1)] = '\0';
  
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sleeping / low power management
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void sleepSeconds(unsigned long seconds) {
    
  softSerialDebug.print(F("sleeping "));
  softSerialDebug.print(seconds);
  softSerialDebug.println(F(" second(s)..."));
  
  unsigned long counterStop = (unsigned long) (seconds / 1.08) + 1;
  
  for(unsigned long counter = 0 ; counter < counterStop; counter++) {
    
    softSerialDebug.println(F("1s"));
    
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    
 }

}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Timestamp computation functions 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned long getTimeStamp(byte year2K, byte month, byte day, byte hour, byte minute, byte second) {
  
  // simplified timestamp computation formula, valid from 2001 to 2099 (year2K = 1->99)
  
  unsigned long timestamp = 978307200;                                                  // 01 Jan 2001 00:00:00 GMT   
  
  timestamp += ((((unsigned long)(year2K-1)) * 365) + (((year2K-1)) / 4))  * 86400;     // timestamp at the beginning of the year
 
  int accuDays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
   
   if(!(year2K % 4)) {
    for(byte i=2 ; i<12 ; i++) accuDays[i] += 1;
  }
 
  timestamp += ((unsigned long)accuDays[month - 1]) * 86400;     // timestamp at the beginning of the month
  
  timestamp += ((unsigned long)day - 1) * 86400;                 // timestamp at the beginning of the day 
  
  timestamp += ((unsigned long)hour) * 3600 + ((int)minute) * 60 + second;

  return timestamp;
  
}



unsigned long getTimeStampNow() {
  
  unsigned long timestampNow = 0;
  
  rtc.getDate();
  rtc.getTime();
  
  byte Y_now = rtc.getYear();
  byte M_now = rtc.getMonth();
  byte D_now = rtc.getDay();
  byte h_now = rtc.getHour();
  byte m_now = rtc.getMinute();
  byte s_now = rtc.getSecond();
  
  timestampNow = getTimeStamp(Y_now, M_now, D_now, h_now, m_now, s_now);
   
  return timestampNow;

}



long getDifferenceInSecondsBetweenDateTimeAndNow(byte year2K, byte month, byte day, byte hour, byte minute, byte second) {
  
  unsigned long timesStampNow = getTimeStampNow();
  
  unsigned long timeStampDateTime = getTimeStamp(year2K, month, day, hour, minute, second);
  
  long difference = (long) timeStampDateTime - (long) timesStampNow;
  
  return difference;
  
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Tasks ordonnancement and scheduling 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void scheduleNextTaskAndSleep() {
  
  unsigned long timestampNow = 0;
  
  rtc.getDate();
  rtc.getTime();
  
  byte Y_now = rtc.getYear();
  byte M_now = rtc.getMonth();
  byte D_now = rtc.getDay();
  byte h_now = rtc.getHour();
  byte m_now = rtc.getMinute();
  byte s_now = rtc.getSecond();
  
  timestampNow = getTimeStamp(Y_now, M_now, D_now, h_now, m_now, s_now);
  
  
  // when will the next READ_SENSORS_AND_STORE_REPORT task occur ?         

  unsigned long nextTASK_READ_SENSORS_AND_STORE_REPORT_timestamp;
  
  byte minutes_READ_SENSORS_AND_STORE_REPORT[12] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
  //byte minutes_READ_SENSORS_AND_STORE_REPORT[60];
  //for(byte i = 0 ; i < 60 ; i++) minutes_READ_SENSORS_AND_STORE_REPORT[i] = i;
  
  byte second_READ_SENSORS_AND_STORE_REPORT = 0;
  
  nextTASK_READ_SENSORS_AND_STORE_REPORT_timestamp = getTimeStamp(Y_now, M_now, D_now, h_now, minutes_READ_SENSORS_AND_STORE_REPORT[0], second_READ_SENSORS_AND_STORE_REPORT) + 3600 ;
  
  for(char i = sizeof(minutes_READ_SENSORS_AND_STORE_REPORT) - 1 ; i>=0 ; i--) {
    
    if((m_now < minutes_READ_SENSORS_AND_STORE_REPORT[i]) or ((m_now == minutes_READ_SENSORS_AND_STORE_REPORT[i]) and (s_now < second_READ_SENSORS_AND_STORE_REPORT))) {
      nextTASK_READ_SENSORS_AND_STORE_REPORT_timestamp = getTimeStamp(Y_now, M_now, D_now, h_now, minutes_READ_SENSORS_AND_STORE_REPORT[i], second_READ_SENSORS_AND_STORE_REPORT);
    }
  
  }
  
  
  // when will the next UPLOAD_STORED_REPORTS task occur ?     
  
  unsigned long nextTASK_UPLOAD_STORED_REPORTS_timestamp;
  
  byte minutes_UPLOAD_STORED_REPORTS[12] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
  
  byte second_UPLOAD_STORED_REPORTS = 30;
  
  nextTASK_UPLOAD_STORED_REPORTS_timestamp = getTimeStamp(Y_now, M_now, D_now, h_now, minutes_UPLOAD_STORED_REPORTS[0], second_UPLOAD_STORED_REPORTS) + 3600 ;
  
  for(char i = sizeof(minutes_UPLOAD_STORED_REPORTS) - 1 ; i>=0 ; i--) {
    
    if((m_now < minutes_UPLOAD_STORED_REPORTS[i]) or ((m_now == minutes_UPLOAD_STORED_REPORTS[i]) and (s_now < second_UPLOAD_STORED_REPORTS))) {
      nextTASK_UPLOAD_STORED_REPORTS_timestamp = getTimeStamp(Y_now, M_now, D_now, h_now, minutes_UPLOAD_STORED_REPORTS[i], second_UPLOAD_STORED_REPORTS);
    }
  
  }
  
  
  // when will the next GPS_ACQUIRE_POSITION_UPDATE_RTC task occur ?      

  unsigned long nextTask_GPS_ACQUIRE_POSITION_UPDATE_RTC_timestamp;
  
  byte hours_GPS_ACQUIRE_POSITION[4] = {1, 7, 13, 19};
  //byte hours_GPS_ACQUIRE_POSITION[24];
  //for(byte i = 0 ; i < 24 ; i++) hours_GPS_ACQUIRE_POSITION[i] = i;
  byte minute_GPS_ACQUIRE_POSITION = 51;
  byte second_GPS_ACQUIRE_POSITION = 50;
  
  nextTask_GPS_ACQUIRE_POSITION_UPDATE_RTC_timestamp = getTimeStamp(Y_now, M_now, D_now, hours_GPS_ACQUIRE_POSITION[0], minute_GPS_ACQUIRE_POSITION, second_GPS_ACQUIRE_POSITION) + 86400 ;

  for(char i = sizeof(hours_GPS_ACQUIRE_POSITION) - 1 ; i>=0 ; i--) {
    
    if((h_now < hours_GPS_ACQUIRE_POSITION[i]) or 
      ((h_now == hours_GPS_ACQUIRE_POSITION[i]) and (m_now < minute_GPS_ACQUIRE_POSITION)) or 
      ((h_now == hours_GPS_ACQUIRE_POSITION[i]) and (m_now == minute_GPS_ACQUIRE_POSITION) and (s_now < second_GPS_ACQUIRE_POSITION))) {
        nextTask_GPS_ACQUIRE_POSITION_UPDATE_RTC_timestamp = getTimeStamp(Y_now, M_now, D_now, hours_GPS_ACQUIRE_POSITION[i], minute_GPS_ACQUIRE_POSITION, second_GPS_ACQUIRE_POSITION);
    }
  
  }
  
 
  // which is the priority task between READ_SENSORS_AND_STORE_REPORT, UPLOAD_STORED_REPORTS and GPS_ACQUIRE_POSITION_UPDATE_RTC ? 
  
  nextTaskTimestamp = nextTASK_READ_SENSORS_AND_STORE_REPORT_timestamp;
  nextTaskID = TASK_READ_SENSORS_AND_STORE_REPORT;
  
  if(nextTASK_UPLOAD_STORED_REPORTS_timestamp < nextTaskTimestamp) {
    
    nextTaskTimestamp = nextTASK_UPLOAD_STORED_REPORTS_timestamp;
    nextTaskID = TASK_UPLOAD_STORED_REPORTS;
    
  }
  
  if(nextTask_GPS_ACQUIRE_POSITION_UPDATE_RTC_timestamp < nextTaskTimestamp) {
    
    nextTaskTimestamp = nextTask_GPS_ACQUIRE_POSITION_UPDATE_RTC_timestamp;
    nextTaskID = TASK_GPS_ACQUIRE_POSITION_UPDATE_RTC;
    
  }
  
  
  sleepSeconds(nextTaskTimestamp - timestampNow);
  
}



void executeScheduledTask() {
  
  boolean success = false;
  
  unsigned long timestampNow = getTimeStampNow();
 
  while((timestampNow > 0) and (timestampNow < nextTaskTimestamp)) {
   
    sleepSeconds(1);  
    
    timestampNow = getTimeStampNow();
    
  }
  
  if(nextTaskID == TASK_READ_SENSORS_AND_STORE_REPORT) {
    
    execute_TASK_READ_SENSORS_AND_STORE_REPORT();
    
  }
  
  else if(nextTaskID == TASK_UPLOAD_STORED_REPORTS) {
    
    execute_TASK_UPLOAD_STORED_REPORTS();
    
  }
  
  else if(nextTaskID == TASK_GPS_ACQUIRE_POSITION_UPDATE_RTC) {
    
    execute_TASK_GPS_ACQUIRE_POSITION_UPDATE_RTC();
    
  }
  
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Tasks definition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void execute_TASK_READ_SENSORS_AND_STORE_REPORT() {
  
  boolean success = readSensorsAndStoreReport();
  
}



void execute_TASK_UPLOAD_STORED_REPORTS() {
  
  float deviceBatteryVoltage = readDeviceBatteryVoltage();
  
  int numOfReportsStored = mStore.getMessagesCount();
  
  if((!POWER_SAVING_OPTION_ACTIVATED) or 
    (POWER_SAVING_OPTION_ACTIVATED and (deviceBatteryVoltage >= POWER_SAVING_REPORTS_UPLOAD_BATTERY_VOLTAGE_TRIGGER_VALUE) or (numOfReportsStored >= POWER_SAVING_REPORTS_UPLOAD_NUM_STORED_MESSAGES_TRIGGER_VALUE))) { 
  
    boolean success = modemPowerOn_httpPostStoredReports_modemPowerOff(1024);
  
  }
  
}



void execute_TASK_GPS_ACQUIRE_POSITION_UPDATE_RTC() {
  
  boolean success = gpsPowerOn_acquireCurrentPosition_updateRTC_gpsPowerOff(GPS_ACQUISITION_HDOP_LIMIT, GPS_ACQUISITION_TIMEOUT_IN_SECONDS);
  
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GPRS activation functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


boolean modemPowerOn_connectToNet() {
    
  if(modem.isOn()) modem.powerOff_On();
  else modem.powerOn();
 
  
  boolean registered = false;
  boolean GPRSAttached = false;
  boolean connectedToNet = false;
    
  modem.activateCommunication();
  

  // is the modem registered ?
  
  if(modem.isCommunicationActivated()) {
    
    modem.configure();
    
    for(byte attempt=0 ; attempt < 60 ; attempt++) {
      
      registered = modem.isRegistered();
      
      if(registered) break;
      
      delay(1000);

    }
    
  }
  
      
  // GPRS attachment
    
  if(registered) {
    
    delay(500);
 
    for(byte attempt=0 ; attempt < 30 ; attempt++) {
      
      GPRSAttached = modem.isGPRSAttached();
      
      if(GPRSAttached) break;
      
      else {
        
        delay(1000);
        modem.attachGPRS();
        
      }
      
    }

  }
    
    
  // PDP context activation
  
  if(GPRSAttached) {
    
    delay(500);
 
    for(byte attempt=0 ; attempt < 30 ; attempt++) {
      
      connectedToNet = modem.isConnectedToNet();
      
      if(connectedToNet) break;
      
      else {
        
        delay(1000);
        
        modem.connectToNet(gprsNetworkAPN, gprsUsername, gprsPassword);
        
      }
      
    }
  
  }
     
      
  return connectedToNet;
  
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// RTC update functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


boolean setRTCDateTime(byte newYear2K, byte newMonth, byte newDay, byte newHour, byte newMinute, byte newSecond) {
  
  boolean success = false;
  
  unsigned long timeStampOfNewDateTime = getTimeStamp(newYear2K, newMonth, newDay, newHour, newMinute, newSecond);
  
  byte year2KProgrammed;
  byte monthProgrammed;
  byte dayProgrammed;
  byte hourProgrammed;
  byte minuteProgrammed;
  byte secondProgrammed;
  
  unsigned long timeStampOfProgrammedDateTime;
  
  for(byte attempt=0 ; attempt < 10 ; attempt++) {
    
    rtc.setTime(newHour, newMinute, newSecond);         // hr, min, sec
    rtc.setDate(newDay, 0, newMonth, 0, newYear2K);   // day, weekday, month, century(1=1900, 0=2000), year(0-99)

    delay(10);
    
    rtc.getDate();
    rtc.getTime();
  
    year2KProgrammed = rtc.getYear();
    monthProgrammed = rtc.getMonth();
    dayProgrammed = rtc.getDay();
    hourProgrammed = rtc.getHour();
    minuteProgrammed = rtc.getMinute();
    secondProgrammed = rtc.getSecond();
  
    timeStampOfProgrammedDateTime = getTimeStamp(year2KProgrammed, monthProgrammed, dayProgrammed, hourProgrammed, minuteProgrammed, secondProgrammed);
    
    long differenceInSeconds = (long)timeStampOfProgrammedDateTime - (long)timeStampOfNewDateTime;
    
    if(abs(differenceInSeconds) <= 1) success = true;
    
    if(success) break;
    
    delay(100);

  }
  
  return success;
   
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Date & time acquisition from server and RTC update functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


boolean modemPowerOn_httpGetTimeInfoFromServer_updateRTC_modemPowerOff() {
  
  boolean rtcUpdated = false;
  
  boolean connectedToNet = modemPowerOn_connectToNet();
  
  if(connectedToNet) {
      
      rtcUpdated = httpGetTimeInfoFromServer_updateRTC();
      
  }
  
  modem.powerOff();
  
  return rtcUpdated;
  
}



boolean httpGetTimeInfoFromServer_updateRTC() {
  
  boolean rtcUpdated = false;
  
  boolean connectedToNet = modem.isConnectedToNet();
  
  if(connectedToNet) {
    
    char timeInfoFromServerBuffer[30];
  
    httpGetTimeInfoFromServer(timeInfoFromServerBuffer, sizeof(timeInfoFromServerBuffer));
          
    rtcUpdated = setRTCFromTimeInfo(timeInfoFromServerBuffer, sizeof(timeInfoFromServerBuffer));

  }
  
  softSerialDebug.print(F("RTC updated from server : "));
  softSerialDebug.println(rtcUpdated);
  
  return rtcUpdated;
    
  
}



void httpGetTimeInfoFromServer(char *timeInfoFromServerBuffer, byte timeInfoFromServerBufferLength) {
  
  boolean tcpConnectSuccess = modem.tcpConnect(serverName, serverPort, 10);
  
  if(tcpConnectSuccess) {
  
    modem.requestAT(F("AT+CIPSPRT=2"), 2, 2000);             // we don't want the "SEND OK" message to be returned after the transmission
    
    modem.requestAT(F("AT+CIPSEND"), 2, 15000);

    modem.echoHttpRequestInitHeaders(serverName, serverGetTimeURL, "GET");
  
    modem.serialConnection.print(F("Connection: close\n"));
    
    modem.serialConnection.print(F("\n"));
  
    modem.serialConnection.print((char) 26);
  
    modem.retrieveHttpResponseBodyFromLineToLine(timeInfoFromServerBuffer, timeInfoFromServerBufferLength, 1, 1, 10000);
  
    modem.tcpClose();
  
  }
  
}



boolean setRTCFromTimeInfo(char *timeInfoFromServerBuffer, byte timeInfoFromServerBufferLength) {
  
  // timeInfoFromServer string example : "GMT : 2013-11-26 16:16:28"
  
  boolean rtcUpdated = false; 
  
  if(strstr(timeInfoFromServerBuffer, "GMT : ") != NULL) {
  
    char subStringBuffer[5];
    
    getSubstr(timeInfoFromServerBuffer, timeInfoFromServerBufferLength, subStringBuffer, sizeof(subStringBuffer), 8, 2);
    byte year2K = atoi(subStringBuffer);
    
    getSubstr(timeInfoFromServerBuffer, timeInfoFromServerBufferLength, subStringBuffer, sizeof(subStringBuffer), 11, 2);
    byte month = atoi(subStringBuffer);
    
    getSubstr(timeInfoFromServerBuffer, timeInfoFromServerBufferLength, subStringBuffer, sizeof(subStringBuffer), 14, 2);
    byte day = atoi(subStringBuffer);
    
    getSubstr(timeInfoFromServerBuffer, timeInfoFromServerBufferLength, subStringBuffer, sizeof(subStringBuffer), 17, 2);
    byte hour = atoi(subStringBuffer);
    
    getSubstr(timeInfoFromServerBuffer, timeInfoFromServerBufferLength, subStringBuffer, sizeof(subStringBuffer), 20, 2);
    byte minute = atoi(subStringBuffer);
    
    getSubstr(timeInfoFromServerBuffer, timeInfoFromServerBufferLength, subStringBuffer, sizeof(subStringBuffer), 23, 2);
    byte second = atoi(subStringBuffer);
    
    rtcUpdated = setRTCDateTime(year2K, month, day, hour, minute, second);
  
  }
  
  return rtcUpdated;
  
}


void getSubstr(char *stringBuffer, byte stringBufferLength, char *subStringBuffer, byte subStringBufferLength, byte indexStart, byte length) {
  
  subStringBuffer[0] = '\0';
  
  for(byte i = 0 ; (i < length) && (i < subStringBufferLength) && ((indexStart + i) < stringBufferLength) ; i++) {
    
    subStringBuffer[i] = stringBuffer[indexStart + i];
    subStringBuffer[i+1] = '\0';
    
  }
  
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Global position acquisition and RTC update functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


boolean gpsPowerOn_acquireCurrentPosition_updateRTC_gpsPowerOff(float accuracyLimit, int timeoutInS) {
  
  boolean positionAcquired = false;
  
  gps.powerOn();
  
  gps.configure();
  
  positionAcquired = gps.acquireNewPosition(accuracyLimit, timeoutInS);
  
  if(positionAcquired and rtcDateTimeSet) {
    
    long differenceInSecondsBetweenLastGPSFixAndNow = getDifferenceInSecondsBetweenDateTimeAndNow(gps.position.fix_Y_utc, gps.position.fix_M_utc, gps.position.fix_D_utc,
                                                                                                  gps.position.fix_h_utc, gps.position.fix_m_utc, gps.position.fix_s_utc);
                                                                                                  
    softSerialDebug.print(F("GPS - RTC diff. : "));
    softSerialDebug.println(differenceInSecondsBetweenLastGPSFixAndNow);                                                                                          
                                                                                                
    if(abs(differenceInSecondsBetweenLastGPSFixAndNow) < 3600) {          // 1 hour
      
      boolean rtcUpdated = setRTCDateTimeFromLastGPSFixData();
      
      softSerialDebug.print(F("RTC updated from GPS : "));
      softSerialDebug.println(rtcUpdated);
      
    }
  
  }
  
  gps.powerOff();
  
  return positionAcquired;
  
}



boolean setRTCDateTimeFromLastGPSFixData() {
  
  boolean rtcUpdated = setRTCDateTime(gps.position.fix_Y_utc, gps.position.fix_M_utc, gps.position.fix_D_utc, gps.position.fix_h_utc, gps.position.fix_m_utc, gps.position.fix_s_utc);
   
  return rtcUpdated;
  
}








/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sensors reading and report storing functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


boolean readSensorsAndStoreReport() {
  
 
  boolean success = false;
  
  char report[REPORT_BUFFER_LENGTH];
  
  char numToCharsBuffer[12];
  
  unsigned long sensorDataAcquisitionTimestamp = getTimeStampNow();
  
  
  // temperature and humidity reading

  float externalTemperature = EXTERNAL_TEMPERATURE_UNDEFINED_VALUE;
  float externalRelativeHumidity = EXTERNAL_RELATIVE_HUMIDITY_UNDEFINED_VALUE;   

  //  float measuredExternalTemperature = temperatureHumiditySensor.readTemperatureC();
  //  float measuredExternalRelativeHumidity = temperatureHumiditySensor.readHumidity();
  //  
  //  if(measuredExternalTemperature > -40) externalTemperature = measuredExternalTemperature;
  //  if(measuredExternalRelativeHumidity > 0) externalRelativeHumidity = measuredExternalRelativeHumidity;
  
  float measuredExternalTemperature;
  float measuredExternalRelativeHumidity;
  
  float accuMeasuredExternalTemperature = 0;
  float accuMeasuredExternalRelativeHumidity = 0;
 
  byte numReadAttemptsOK = 0;
   
  delay(2000);                                          
  
  for(byte i=0 ; i < 3 ; i++) {      
  
    measuredExternalTemperature = temperatureHumiditySensor.readTemperatureC();
    measuredExternalRelativeHumidity = temperatureHumiditySensor.readHumidity();
    
    if((measuredExternalTemperature > -40) and (measuredExternalRelativeHumidity > 0)) {
      
      numReadAttemptsOK++;
    
      accuMeasuredExternalTemperature += measuredExternalTemperature;
      accuMeasuredExternalRelativeHumidity += measuredExternalRelativeHumidity;
    } 
    
    delay(2000);
   
  }

  if(numReadAttemptsOK > 0) {
    
    externalTemperature = accuMeasuredExternalTemperature / numReadAttemptsOK;
    externalRelativeHumidity = accuMeasuredExternalRelativeHumidity / numReadAttemptsOK;
    
  }
  
  
  
  // pressure reading
  
  float pressure = PRESSURE_UNDEFINED_VALUE;
  
  float measuredPressure = pressureSensor.readPressure() / 100.0;
   
  if((measuredPressure > 900.0) and (measuredPressure < 1100.0)) {
     
   pressure = measuredPressure;
     
  }


  // wind / rain meter reading
  
  windRainMeter.getReport();
  
  byte windRainMeterReportID = windRainMeter.reportId;
  
  int windRainMeterElapsedTimeSinceLastReportInSeconds = windRainMeter.elapsedTimeSinceLastReportInSeconds;
  

  // device temperature reading
  
  float deviceTemperature = DEVICE_TEMPERATURE_UNDEFINED_VALUE;
  
  float measuredDeviceTemperature = pressureSensor.readTemperature();
  
  if(measuredDeviceTemperature > DEVICE_TEMPERATURE_UNDEFINED_VALUE) deviceTemperature = measuredDeviceTemperature;
  
  
  // device battery voltage reading
  
  float deviceBatteryVoltage = readDeviceBatteryVoltage();
  
  
  // report construction

  char reportFormatIdentifier[] = "A";         // hex string

  char sep[2] = ",";
  
 
  strcpy(report, reportFormatIdentifier);
  
  strcat(report, sep);
  
  strcat(report, stationID);
  
  strcat(report, sep);
  
  strcat(report, firmwareVersion);
  
  strcat(report, sep);
  
  if(sensorDataAcquisitionTimestamp != 0) strcat(report, ultoa(sensorDataAcquisitionTimestamp, numToCharsBuffer, 10));
           
  strcat(report, sep);

  if(externalTemperature != EXTERNAL_TEMPERATURE_UNDEFINED_VALUE) {
   
    strcat(report, dtostrf(externalTemperature, 1, 1, numToCharsBuffer));
    
  }

  strcat(report, sep);
  
  if(externalRelativeHumidity != EXTERNAL_RELATIVE_HUMIDITY_UNDEFINED_VALUE) {
    
    strcat(report, dtostrf(externalRelativeHumidity, 1, 0, numToCharsBuffer));
    
  }
  
  strcat(report, sep);
  
  if(pressure != PRESSURE_UNDEFINED_VALUE) {
    
    strcat(report, dtostrf(pressure, 1, 1, numToCharsBuffer));
    
  }
  
  
  if((RAIN_GAUGE_SENSOR_CONNECTED_AND_ACTIVATED or ANEMOMETER_SENSOR_CONNECTED_AND_ACTIVATED) and (windRainMeterReportID != 255) and (windRainMeterElapsedTimeSinceLastReportInSeconds > 0)) {
    
    int elapsedTimeSinceLastReportInSeconds = sensorDataAcquisitionTimestamp - previousReportTimeStamp;
    
    if(abs(windRainMeterElapsedTimeSinceLastReportInSeconds - elapsedTimeSinceLastReportInSeconds) <= 4)  {
      windRainMeterElapsedTimeSinceLastReportInSeconds = elapsedTimeSinceLastReportInSeconds; 
    }
                
    strcat(report, sep);
    
    strcat(report, itoa(windRainMeterElapsedTimeSinceLastReportInSeconds, numToCharsBuffer, 10));
    
    strcat(report, sep);
    
    if(RAIN_GAUGE_SENSOR_CONNECTED_AND_ACTIVATED) {
      
      strcat(report, dtostrf(windRainMeter.accumulatedRainfallMM, 1, 1, numToCharsBuffer));
      
    }
      
      
    if(ANEMOMETER_SENSOR_CONNECTED_AND_ACTIVATED) {
      
      strcat(report, sep);
            
      strcat(report, dtostrf(windRainMeter.meanWindSpeedMetersPerSecond, 1, 1, numToCharsBuffer));
      
      strcat(report, sep);
      
      strcat(report, itoa(windRainMeter.meanWindDirectionDecimalDegrees, numToCharsBuffer, 10)); 
      
      strcat(report, sep);
      
      strcat(report, dtostrf(windRainMeter.maxWindSpeedMetersPerSecond, 1, 1, numToCharsBuffer));
      
    }
    
    else {
      
     for(byte i = 0 ; i < 3 ; i++) strcat(report, sep);
      
    }
    
  }
  
  else {
   
   for(byte i = 0 ; i < 5 ; i++) strcat(report, sep);
    
  }

  if(gps.firstPositionAcquired) {

    strcat(report, sep);
    
    unsigned long fixPositionTimestamp = getTimeStamp(gps.position.fix_Y_utc, gps.position.fix_M_utc, gps.position.fix_D_utc, gps.position.fix_h_utc, gps.position.fix_m_utc, gps.position.fix_s_utc);

    strcat(report, ultoa((sensorDataAcquisitionTimestamp - fixPositionTimestamp), numToCharsBuffer, 10));
      
    strcat(report, sep);

    strcat(report, dtostrf(gps.position.latitude, 1, 4, numToCharsBuffer));
      
    strcat(report, sep);
      
    strcat(report, dtostrf(gps.position.longitude, 1, 4, numToCharsBuffer));
      
    strcat(report, sep);
      
    strcat(report, itoa(gps.position.altitudeAboveMSL, numToCharsBuffer, 10)); 
    
  }
  
  else {
   
   for(byte i = 0 ; i < 4 ; i++) strcat(report, sep);
    
  }
 
 
   strcat(report, sep);
  
  if(deviceTemperature != DEVICE_TEMPERATURE_UNDEFINED_VALUE) {
    
    strcat(report, dtostrf(deviceTemperature, 1, 0, numToCharsBuffer));
    
  }
  
  strcat(report, sep);
  
  if(deviceBatteryVoltage != BATTERY_VOLTAGE_UNDEFINED_VALUE) {
    
    strcat(report, dtostrf(deviceBatteryVoltage, 1, 2, numToCharsBuffer));
    
  }
  
  strcat(report, sep);
  
  strcat(report, itoa(reportCounter, numToCharsBuffer, 10)); 
 
 
  strcat(report, sep);
    
  strcat(report, itoa(windRainMeterReportID, numToCharsBuffer, 10));        
  
 
  // report storing
       
  softSerialDebug.println(report);
       
  success = mStore.storeMessage(report);
  
  previousReportTimeStamp = sensorDataAcquisitionTimestamp;
  
  reportCounter++;
  if(reportCounter == 255)  reportCounter = 0;
  
  return success;

 }



float readDeviceBatteryVoltage() {

  float deviceBatteryVoltage = BATTERY_VOLTAGE_UNDEFINED_VALUE;
  
  unsigned long accuAdcReadings = 0;
    
  for(byte i=0 ; i < 10; i++) {
    accuAdcReadings += analogRead(DEVICE_BATTERY_VOLTAGE_PIN);
  }
  
  if(accuAdcReadings > 0) deviceBatteryVoltage = accuAdcReadings * ADC_REF_VOLTAGE * 156.0 / (10 * 1024 * 56.0);
  
  return deviceBatteryVoltage;

}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Reports uploading functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
boolean modemPowerOn_httpPostStoredReports_modemPowerOff(int maxNumOfReportsToBeSent) {
  
  boolean success = false;
  
  // are there any stored reports to be sent ?
  
  int numOfReportsStored = mStore.getMessagesCount();
  
  if(numOfReportsStored > 0) {
    
    boolean connectedToNet = modemPowerOn_connectToNet();
  
    if(connectedToNet) success = httpPostStoredReports(maxNumOfReportsToBeSent);

    modem.powerOff();
  
  }
  
  else success = true;
  
  return success;
  
}
    
    

boolean httpPostStoredReports(int maxNumOfReportsToBeSent) {  
    
  boolean success = false;
  
  // are there any stored reports to be sent ?

  int numOfReportsStored = mStore.getMessagesCount();
    
  if(numOfReportsStored == 0) success = true;
  
  else {
    
    boolean connectedToNet = modem.isConnectedToNet();
    
    if(connectedToNet) {
    
      delay(500);
      
      
      // which is the real number of reports to be sent, and which is the corresponding total file content length ?
      
      int numOfReportsToBeSent = min(numOfReportsStored, maxNumOfReportsToBeSent);
      
      int reportsCounter = 0;
 
      byte reportLength;
      long totalContentLength = 0;    
   

      for(int pageIndex = 0 ; (pageIndex < 1024) && (reportsCounter < numOfReportsToBeSent) ; pageIndex++) {
        
        reportLength = mStore.getMessageLength(pageIndex);
        
        if(reportLength > 0) {
          
          totalContentLength += (reportLength + 2);     // "\r\n" will be sent after each report 
          reportsCounter++;
          
        }
        
      }
      
      
      byte maxNumOfReportsForOneBlockTransmission = 6;          
      byte maxNumOfReportsPerTransmissionBlock = 10;

      char report[REPORT_BUFFER_LENGTH];

      char incomingCharsBuffer[60];
      
      
      // TCP connection and request transmission
      
      boolean tcpConnectSuccess = modem.tcpConnect(serverName, serverPort, 10);
      
      if(tcpConnectSuccess) {
        
        if(numOfReportsToBeSent <= maxNumOfReportsForOneBlockTransmission) {                            
          
          modem.requestAT(F("AT+CIPSPRT=2"), 2, 2000);             // we don't want the "SEND OK" message to be returned after each transmission
          
          modem.requestAT(F("AT+CIPSEND"), 2, 15000);
          
          modem.echoHttpRequestInitHeaders(serverName, serverPostReportsURL, "POST");
          
          modem.serialConnection.print(F("Connection: close\n"));
          
          modem.echoHttpPostFileRequestAdditionalHeadersPart1(totalContentLength, serverPostReportsFileFormFieldName);
          
          reportsCounter = 0;
          
          for(int pageIndex = 0 ; (pageIndex < 1024) && (reportsCounter < numOfReportsToBeSent); pageIndex++) {
            
            reportLength = mStore.getMessageLength(pageIndex);
            
            if(reportLength > 0) {
              
              mStore.retrieveMessage(pageIndex, report);
              
              modem.serialConnection.print(report);
              modem.serialConnection.print("\r\n");
              
              reportsCounter++;
              
            }
            
          }
          
          modem.echoHttpPostFileRequestAdditionalHeadersPart2();
          
          modem.serialConnection.print((char) 26);
          
        }
        
          
        else {      
            
          boolean sendError = false;
          
          modem.requestAT(F("AT+CIPSPRT=1"), 2, 2000);             // we want the "SEND OK" message to be returned after each transmission, in order to check that everything was OK 
  
          modem.requestAT(F("AT+CIPSEND"), 2, 15000);
          
          modem.echoHttpRequestInitHeaders(serverName, serverPostReportsURL, "POST");
          modem.echoHttpPostFileRequestAdditionalHeadersPart1(totalContentLength, serverPostReportsFileFormFieldName);
          
          modem.serialConnection.print((char) 26);
          
          modem.retrieveIncomingCharsFromLineToLine(incomingCharsBuffer, sizeof(incomingCharsBuffer), 0, 1, 30000);
          if(strstr(incomingCharsBuffer, "OK") == NULL) sendError = true;
                   
      
          delay(300);
          
          
          reportsCounter = 0;
  
          for(int pageIndex = 0 ; (pageIndex < 1024) && (reportsCounter < numOfReportsToBeSent) && !sendError; pageIndex++) {
            
            reportLength = mStore.getMessageLength(pageIndex);
            
            if(reportLength > 0) {
              
              if((reportsCounter % maxNumOfReportsPerTransmissionBlock) == 0) {
                
                delay(300);
                
                modem.requestAT(F("AT+CIPSEND"), 2, 15000);
                
              }
                
              mStore.retrieveMessage(pageIndex, report);
              
              modem.serialConnection.print(report);
              modem.serialConnection.print("\r\n");
              
              
              if(((reportsCounter % maxNumOfReportsPerTransmissionBlock) == (maxNumOfReportsPerTransmissionBlock - 1)) || (reportsCounter == (numOfReportsToBeSent - 1))) {
                
                modem.serialConnection.print((char) 26);
                
                modem.retrieveIncomingCharsFromLineToLine(incomingCharsBuffer, sizeof(incomingCharsBuffer), 0, 1, 30000);                 
                if(strstr(incomingCharsBuffer, "OK") == NULL) sendError = true;
                
              }
              
              reportsCounter++;
              
            }
            
          }
          
          
          if(!sendError) {
        
            delay(300);
          
            modem.requestAT(F("AT+CIPSPRT=2"), 2, 2000);             // we don't want anymore the "SEND OK" message to be returned after each transmission, in order to get only the server's response after the last one
            
            modem.requestAT(F("AT+CIPSEND"), 1, 15000);
            modem.echoHttpPostFileRequestAdditionalHeadersPart2();
            modem.serialConnection.print((char) 26);
            
          }
             
        }
          
        
       // server's response interpretation
        
        //modem.retrieveHttpResponseStatusLine(incomingCharsBuffer, sizeof(incomingCharsBuffer), 90000);
        
        //if(strstr(incomingCharsBuffer, "200") != NULL) success = true;
        
        modem.retrieveHttpResponseBodyFromLineToLine(incomingCharsBuffer, sizeof(incomingCharsBuffer), 1, 1, 90000);     // the timeout must be long enough for the server to respond after the "ingestion" of tens of reports

        softSerialDebug.print(F("Resp : "));
        softSerialDebug.println(incomingCharsBuffer);
        
        if(strstr(incomingCharsBuffer, "OK") != NULL) success = true;

                
        modem.tcpClose();
        
        
        // in case of success, we delete the corresponding reports in the store
         
        if(success) {
          
          reportsCounter = 0;
         
          for(int pageIndex = 0 ; (pageIndex < 1024) && (reportsCounter < numOfReportsToBeSent) ; pageIndex++) {
            
            reportLength = mStore.getMessageLength(pageIndex);
            
            if(reportLength > 0) {
              
              mStore.clearPage(pageIndex);
              
              reportsCounter++;
              
            }
            
          }
          
        }  
        
        
      }
    
    }
    
  }

  return success;

}  
    



