
/*
 *
 * Position acquisition demonstration 
 *
 * The Ultimate_GPS library assumes that the Arduino communicates with the Ultimate GPS module via it's hardware serial port
 * and that a dedicated pin of the microcontroller is used to control the power status of the GPS module.
 *
 * Arduino pin 0 <-> Ultimate GPS module TX
 * Arduino pin 1 <-> Ultimate GPS module RX
 * Arduino pin 4 <-> Ultimate GPS module EN
 *
 * The Arduino can't print debugging data to the hardware serial port as it is already used for the communication with the
 * GPS module. This data thus must be sent to a software serial port (here, software TX on Arduino pin 3). 
 *
 * The GPS module is powered ON at the beginning of the sketch and powered OFF at the end.
 *
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
*/



#include "SoftwareSerial.h"

#include "Ultimate_GPS.h"



// pins definition

#define GPS_POWER_PIN 4

#define DEBUG_TX_PIN 3
#define DEBUG_RX_PIN 255


// Test configuration

#define GPS_ACQUISITION_TIMEOUT_IN_SECONDS 300

#define GPS_ACQUISITION_HDOP_LIMIT 10





SoftwareSerial softSerial(DEBUG_RX_PIN, DEBUG_TX_PIN); 

UltimateGPS gps(&Serial, GPS_POWER_PIN, &softSerial);




void setup()  {
  
  softSerial.begin(9600);
  
  gps.init(9600);
  
  softSerial.println("Going on...\n");
  
  gps.powerOn();
  
  delay(1000);
  
  softSerial.print("Test configuration : \n");
  
  softSerial.print("Timeout : ");
  softSerial.print(GPS_ACQUISITION_TIMEOUT_IN_SECONDS);
  softSerial.println(" seconds");
  
  softSerial.print("HDOP (horizontal dilution of precision) limit : ");
  softSerial.println(GPS_ACQUISITION_HDOP_LIMIT);
  
  softSerial.println();
  
  softSerial.print("Starting position acquisition (please wait, ");
  softSerial.print(GPS_ACQUISITION_TIMEOUT_IN_SECONDS);
  softSerial.println(" seconds max...)\n");
  
  unsigned long millisAcquisitionStart = millis();
  
  boolean positionAcquired = gps.acquireNewPosition(10, 600);
  
  if(positionAcquired) {
    
    unsigned long millisAcquisitionEnd = millis();
    
    softSerial.print("Position acquired in ");
    softSerial.print((millisAcquisitionEnd - millisAcquisitionStart) / 1000.0);
    softSerial.println(" seconds.\n");
    
    softSerial.print("Latitude : ");
    softSerial.print(gps.position.latitude, 5);
    softSerial.println(" decimal degrees");
  
    softSerial.print("Longitude : ");
    softSerial.print(gps.position.longitude, 5);
    softSerial.println(" decimal degrees");
  
    softSerial.print("Altitude above MSL : ");
    softSerial.print(gps.position.altitudeAboveMSL, 1);
    softSerial.println(" meters");
    
    softSerial.print("HDOP : ");
    softSerial.println(gps.position.horizontalDilutionOfPrecision);
    
  }
  
  else {
    
    softSerial.println("Position was NOT acquired (timeout reached) !!!\n");
    
  }
  
  gps.powerOff();
  
}



void loop() {
  

}

