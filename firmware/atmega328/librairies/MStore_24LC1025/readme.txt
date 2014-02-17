### 24LC1025 EEPROM "store" interface library for Arduino

This library implements a virtual "store" based on the 24LC1025 chip in which "messages" 
(usually ASCII strings) can be recorded in and retrieved from the EEPROM.

Each store has an "address" corresponding to its I2C address, depending of the configuration of 
the A0 and A1 pins of the 24LC1025 chip (for example, 0x53 if A0 and A1 are tied to VCC).

A store contains 1024 pages in which messages from variable length up to 124 bytes can be recorded.

Each page is identified by an "index" (from 0 to 1023) and is associated to a "writes counter" 
corresponding to the number of previously written (and erased) messages in this page. 

The writes counter aims to give a representation of the page wear : when the writes counter of a page
exceeds a predefined value (MAX_NUM_WRITES_PER_PAGE, set in the MStore_24LC1025.h file), the page is 
considered as unreliable and it won't be possible to write any more message on it.  

Please note that this counter only makes sense if you start to work with a new chip : the writes counter 
does not of course reflect the REAL state / wear of a previously heavily used chip.

A page may contain 0 or 1 message which length can not exceed 124 bytes (so a user can at most 
record 1024 messages in the store). 

A message can be stored, retrieved, or deleted with the help of several methods, by setting the 
corresponding page index as parameter.
 
 
Dependencies : Wire library.


Author : Previmeteo (www.previmeteo.com)

Project web site : http://oses.previmeteo.com/

License: GNU GPL v2 (see License.txt)