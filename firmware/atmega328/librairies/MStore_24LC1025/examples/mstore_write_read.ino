/*
 * MStore_24LC1025 write and read demonstration
 * 
 * We suppose here that the A0 and A1 pins of the 24LC1025 chip are tied to VCC, which gives it
 * the address : 0x53
 *
 * This sketch stores 115 messages in the EEPROM and shows how to retrieve them later.
 *
 * Please note that you will have to "format" the EEPROM at the beginning of the sketch if you are 
 * using a new chip or a previously used one which may contain residual data (uncomment the line 
 * mStore.smashAllPages() below if required)
 *
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
*/


#include <Wire.h>

#include <MStore_24LC1025.h>


#define EEPROM_STORE_ADDRESS 0x53


MStore_24LC1025 mStore(EEPROM_STORE_ADDRESS);


char msgPart1[] = "I was born on ";
char msgPart2[] = ", and you ?";

char msg[125];

char numToCharsBuffer[12];

  

void setup(void) {
  
  Serial.begin(9600);
  
  Wire.begin();  
  
  Serial.println("Going on...\n");
 
  mStore.init();
 
  delay(5000);
 

//  Serial.println("smashing pages (this may take one ot two minutes, please wait !)...");
//  
//  mStore.smashAllPages();    // This will "format" the chip by writing zeros on every byte of the EEPROM, 
//                                // and thus prepare the store to record its first messages
//                                
//                                // Required if you want to test this sketch on a new chip or a previously used chip
//                                //  which may contain residual data
//                                
// Serial.println("... end of smashing !!!");                
           

  
  Serial.print("messages count : ");
  Serial.println(mStore.getMessagesCount());
  Serial.println();
  
  
  delay(1000);
  
  
  for(int i = 0 ; i < 115 ; i++) {
    
    Serial.print("storing one more message : ");
    Serial.print(i);
    Serial.print(" / 114...");
    
    msg[0] = '\0';
    
    strcpy(msg, msgPart1);
    strcat(msg, itoa((1900 + i), numToCharsBuffer, 10));
    strcat(msg, msgPart2);
    
    boolean success = mStore.storeMessage(msg);
    
    if(success) Serial.println(" : success !");
    else Serial.println(" : error !!!");
    
  }
  
  
  delay(1000);
 
  
  Serial.print("\nmessages count is now : ");
  Serial.println(mStore.getMessagesCount());
  Serial.println();
  
  
  delay(1000);
  
  
  for(int i = 0 ; i < 1024 ; i++) {  
    
    byte msgLength = mStore.getMessageLength(i);
    
    if(msgLength) {
    
      mStore.retrieveMessage(i, msg);
      Serial.print("message stored on page  ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(msg);
    
    }
  
  }
  
}

 
 
void loop() {
  
}
 
 
 

