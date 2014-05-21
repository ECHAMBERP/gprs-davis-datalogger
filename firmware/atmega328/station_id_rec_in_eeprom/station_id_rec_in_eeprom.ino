/*
 *
 * Description :
 *
 * This sketch writes a string of 8 characters in the first 8 bytes of the internal EEPROM of the microcontroller.
 *
 * This string will be used later as the station's unique identifier.
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
 * Version : 1.2
 *
 * Creation date : 2014/05/21
 * 
 */
 
 
 
#include <EEPROM.h>


#define STATION_ID "MY_IDENT"     // string length MUST be equal to (STATION_ID_BUFFER_LENGTH - 1), so here 8 characters


#define INTERNAL_EEPROM_STATION_ID_ADDRESS_START 0

#define STATION_ID_BUFFER_LENGTH 9





void setup() {

  Serial.begin(9600);

  delay(3000);
  
  Serial.println("Writing station ID...");
  
  writeStationIdInInternalEEPROM();
  
  delay(1000);

  char retrievedStationID[STATION_ID_BUFFER_LENGTH];
  
  retrieveStationIdFromInternalEEPROM(retrievedStationID);
  
  Serial.print("station ID recorded in EEPROM is : ");
  Serial.println(retrievedStationID);
  
}



void loop() {


}



void writeStationIdInInternalEEPROM() {
  
  char stationID[STATION_ID_BUFFER_LENGTH] = STATION_ID;
  
  for(byte i = 0 ; i < (STATION_ID_BUFFER_LENGTH - 1) ; i++) {
    
    EEPROM.write(INTERNAL_EEPROM_STATION_ID_ADDRESS_START + i, stationID[i]);
    
  }
  
}



void retrieveStationIdFromInternalEEPROM(char stationID[STATION_ID_BUFFER_LENGTH]) {
    
  for(byte i = 0 ; i < (STATION_ID_BUFFER_LENGTH - 1) ; i++) {
    
   stationID[i] = EEPROM.read(INTERNAL_EEPROM_STATION_ID_ADDRESS_START + i);
   
  }
  
  stationID[(STATION_ID_BUFFER_LENGTH - 1)] = '\0';
  
}

