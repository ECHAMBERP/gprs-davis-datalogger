/*
*
* HTTP GET request demonstration (example of "manual" request construction)
*
* The Arduino is connected to the GPRSBee via it's pins 5, 6, 7 and 8 and thus requires the use of the SoftwareSerial librairy
* for the communication with the modem. 
* 
* Arduino pin 5 <-> GPRSBee pin 9 (DTR)
* Arduino pin 6 <-> GPRSBee pin 3 (DIN)
* Arduino pin 7 <-> GPRSBee pin 2 (DOUT)
* Arduino pin 8 <-> GPRSBee pin 12 (CTS)
*
* An HTTP GET request is built "manually" in order to get the modem's public IP from the ipecho.net server.
*
* The modem is powered on at the beginning of the sketch and powered off after the request.
*
* Don't forget to edit the GPRS_NETWORK_APN, GPRS_USERNAME and GPRS_PASSWORD values below !!!
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

#include "GPRSbee.h"


// pins definition

#define MODEM_POWER_PIN 5
#define MODEM_TX_PIN 6
#define MODEM_RX_PIN 7
#define MODEM_STATUS_PIN 8


// GPRS connection parameters

#define GPRS_NETWORK_APN "your_apn"
#define GPRS_USERNAME "your_username"
#define GPRS_PASSWORD "your_password"   



GPRSbee modem(MODEM_POWER_PIN, MODEM_STATUS_PIN, MODEM_RX_PIN, MODEM_TX_PIN);



void setup()  {
  
  Serial.begin(9600);
  
  modem.init(4800);
  
  Serial.println("Going on...\n");

  delay(5000);     // required to obtain a reliable response from the modem.isOn() call below
 
  if(modem.isOn()) modem.powerOff();
  
  delay(2000);
  
  
  boolean connectedToNet = modemPowerOn_connectToNet();
  
  if(connectedToNet) getAndPrintPublicIP();
  
  modem.powerOff();
  
  Serial.println("Modem is off !!!");
  
  
}
  


void loop() {
  
}




boolean modemPowerOn_connectToNet() {
    
  if(modem.isOn()) modem.powerOff_On();
  else modem.powerOn();
 
  delay(500);
 
  if(modem.isOn()) Serial.println("Modem is on !!!\n");
  
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
  
  if(registered) Serial.println("Modem registered !!!");
  else Serial.println("Modem was NOT registered !!!");
      
      
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
    
  if(GPRSAttached) Serial.println("GPRS attached !!!");
  else Serial.println("GPRS attachment error !!!");
      
    
  // PDP context activation
  
  if(GPRSAttached) {
    
    delay(500);
 
    for(byte attempt=0 ; attempt < 30 ; attempt++) {
      
      connectedToNet = modem.isConnectedToNet();
      
      if(connectedToNet) break;
      
      else {
        
        delay(1000);
        
        modem.connectToNet(GPRS_NETWORK_APN, GPRS_USERNAME, GPRS_PASSWORD);
        
      }
      
    }
  
  }
  
  if(connectedToNet) Serial.println("Connected to Net !!!\n");
  else Serial.println("Connection error (please check your APN, username and password) !!!\n");
     
  return connectedToNet;
  
}




boolean getAndPrintPublicIP() {
   
  boolean tcpConnectSuccess = modem.tcpConnect("ipecho.net", "80", 10);
  
  if(tcpConnectSuccess) {
    
    char responseBuffer[60];
  
    modem.requestAT("AT+CIPSEND", 2, AT_CIPSEND_RESP_TIMOUT_IN_MS);

    modem.echoHttpRequestInitHeaders("ipecho.net", "/plain", "GET");
  
    modem.serialConnection.print("Connection: close\n");
    
    modem.serialConnection.print('\n');
  
    modem.serialConnection.print((char) 26);
    
    modem.retrieveHttpResponseBodyFromLineToLine(responseBuffer, sizeof(responseBuffer), 1, 1, HTTP_RESP_TIMOUT_IN_MS);
    
    Serial.print("Public IP is : ");
    
    Serial.println(responseBuffer);
  
    modem.tcpClose();
  
  }
  
  else Serial.println("Could not contact ipecho.net server !!!");
   
}

