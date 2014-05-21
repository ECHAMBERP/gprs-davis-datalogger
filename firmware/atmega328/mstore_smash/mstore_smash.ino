/*
 *
 * Description :
 *
 * This sketch "smashes" the content of the 24LC1025 chip by writing zeros on every byte of the EEPROM.
 *
 * This will prepare the "store" to record its first messages, and is required in any case if you're using 
 * a new chip or a previously used one which may contain residual data.
 *
 * Please note that this "smashing" not only deletes every "messages" which may be already stored in the EEPROM
 * but also (re)sets to 0 the "writes counter" of the 1024 pages of the store.
 *
 * We suppose here that the A0 and A1 pins of the 24LC1025 chip are tied to VCC, which gives it
 * the address : 0x53
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
 

#include "Wire.h"
 
#include "MStore_24LC1025.h"


#define EEPROM_STORE_ADDRESS 0x53       // A0 and A1 pins of the 24LC1025 chip are tied to VCC


MStore_24LC1025 mStore(EEPROM_STORE_ADDRESS);



void setup() {

  Serial.begin(9600);
  
  Wire.begin();  
 
  mStore.init();
  
  Serial.println("Going on...\n");
  
  delay(10000);
  
  for(int pageIndex = 0 ; pageIndex < 1024 ; pageIndex++) {
    
    Serial.print("Smashing page ");
    Serial.print(pageIndex);
    Serial.println(" ...");
    
    mStore.smashPage(pageIndex);     
                                                        
  }
  
  Serial.print("messages count is now : ");
  Serial.println(mStore.getMessagesCount());
  
  //Serial.println("Dumping store content...");
  //
  //for(int i = 0 ; i < 1024 ; i++) {    
  //  mStore.dumpPageHuman(i);
  //}
  
}



void loop() {


}



