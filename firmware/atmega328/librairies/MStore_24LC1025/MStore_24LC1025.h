/*
 * File : MStore_24LC1025.h
 *
 * Version : 1.2
 *
 * Purpose : 24LC1025 EEPROM "store" interface library for Arduino
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
 * Creation date : 2014/05/21
 *
 * History :
 *
 * - 1.2 : prototype changes on the write and store functions
 *         and addition of new functions : storeMessageCheck() and clearPageCheck()
 * 
 */




#ifndef MSTORE_24LC1025_h
#define MSTORE_24LC1025_h



#include "Arduino.h"

#include "Wire.h"




#define MAX_CHUNK_SIZE 16

#define MAX_NUM_WRITES_PER_PAGE 100000




class MStore_24LC1025 {


  public:
  
    MStore_24LC1025(byte storeAddress);
    
    void init();
    
    void writeMessage(int pageIndex, char* message);

    int storeMessage(char* message);
    
    boolean storeMessageCheck(char* message);
    
    int getMessagesCount();
    
    unsigned long getWritesCount(int pageIndex);
    
    byte getMessageLength(int pageIndex);
    
    void retrieveMessage(int pageIndex, char* message);

    void clearPage(int pageIndex);
    
    boolean clearPageCheck(int pageIndex);
    
    void clearAllPages();
    
    void smashPage(int pageIndex);
    
    void smashAllPages();
    
    void dumpPageHex(int pageIndex);
    
    void dumpPageHuman(int pageIndex);
    
    
  private:
  
    byte _storeAddress;
  
    byte getTwiAddress(int pageIndex);
    
    unsigned int getPageStartAddress(int pageIndex);
    

};



#endif