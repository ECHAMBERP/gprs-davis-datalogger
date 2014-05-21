
### ATMEGA328 programming


The ATMEGA328 which sits on the board can be programmed "in situ" with the ARDUINO IDE, using an AVR ISP programmer 
and an FTDI cable (3.3 V version).


There are several steps required to setup the IDE, format the EEPROMs used on the board and upload the firmware.



A/ IDE setup and bootloaders / librairies installation


- download and install the Arduino 1.0.x IDE from http://arduino.cc/en/main/software (tested version : 1.0.4)


- create (if not already done) two sub-directories in your Sketchbook directory, respectively named 'hardware' and
  'librairies'
  
  
- copy the 'bootloaders/hardware/the_bootloaders' directory in the previously created 'hardware' directory 


- copy the Ultimate_GPS library in the previously created 'librairies' directory (version : 1.2) 


- copy the MStore_24LC1025 library in the librairies directory (version : 1.2) 


- copy the GPRSbee library in the librairies directory (version : 1.2) 


- copy the WindRainMeter_DavisSensors library in the librairies directory (version : 1.2) 


- download the LowPower library from https://github.com/rocketscream/Low-Power and copy it to libraries/LowPower 
  (tested version : 1.30)


- download the Rtc_Pcf8563 library from http://playground.arduino.cc/Main/RTC-PCF8563  and copy it to 
  libraries/Rtc_Pcf8563 (tested version : 1.0.1)


- download the Adafruit_BMP085 library from https://github.com/adafruit/Adafruit-BMP085-Library and copy it to 
  libraries/Adafruit_BMP085 (tested revision : 4802c1b8d3)


- download the SHT1x library from https://github.com/practicalarduino/SHT1x and copy it to libraries/SHT1x 
  (tested revision : be7042c3e3)


You should end up with with the following structure inside your 'Sketchbook' directory :


    + Sketchbook
    
      + hardware
    
        + the_bootloaders
    
          + bootloaders
    
            + optiboot
            + standard
            
      + librairies
    
        + Adafruit_BMP085
        + GPRSbee
        + LowPower
        + MStore_24LC1025
        + Rtc_Pcf8563
        + SHT1x
        + Ultimate_GPS
        + WindRainMeter_DavisSensors
    



B/ optiboot bootloader upload on the ATMEGA328


The "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328 -> optiboot" bootloader needs to be uploaded 
on the ATMEGA328 in order to allow it to be (re)programmed later with the Arduino IDE and an FTDI cable. 

The optiboot bootloader is required here because the GPRS datalogger firmware (gprs_datalogger.ino),
which we will upload later on the Atmega, is too big in size to be used with the standard "Arduino Pro or Pro Mini
(3.3V, 8 MHz) w/ ATmega328" bootloader, while there still remains some space on the chip when using the optiboot.
   
The upload process of the optiboot bootloader is very easy, provided that you copied the required files
in your 'Sketchbook' directory during the previous stage and own an AVR ISP programmer. 

If you don't have one, you can easily make it using an Arduino module as in this tutorial :

http://letsmakerobots.com/node/35649 


With this programmer, you will be able to upload the optiboot bootloader on the board's ATMEGA328.

For this, connect your programmer to the ISP connector on the board, launch your Arduino IDE and :

- go int the Tools > Programmer menu and select the type of programmer (for example : "Arduino as ISP") 

- go int the Tools > Board menu and select "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328 -> optiboot"

- go in the Tools menu and select "Burn bootloader"  


The upload process should last about 1 minute. Once this is done done, you will be able to reprogram 
the ATMEGA328 on the board using a "regular" FTDI cable, 3.3V version, and won't need anymore the 
AVR ISP programmer.

Please note however that you will need to ensure that * the GPS module included on the board is OFF *
each time you will try to upload a new sketch with the FTDI cable. 

This is required because the GPS module is linked to the hardware serial port of the ATMEGA328, which
is also connected to the FTDI cable. It is NOT possible to use the FTDI cable if the GPS is powered
ON because the later "spits" away NMEA strings and thus prevents the ATMEGA328 to be reprogrammed via
the serial link. 

There are two ways to power OFF temporary the GPS module during reprogramming operations :

- put the GPS's power switch in the OFF position
- or unplug the GPS module (if you didn't solder it on the board, but just plugged it on a socket)




C/ 24LC1025 EEPROM chip formatting

The 24LC1025 EEPROM chip ("external EEPROM") included in the schematic of the board is used to store 
temporary data collected by the station before sending it to a web server.

This chip must be "formatted" in a special way (zeros written on each byte of the EEPROM) before it
can be used to store the first reports.

The chip formatting can be done with the help of a dedicated Arduino sketch, which you will have to
upload an run on the board.

For this :

- put the GPS's power switch in the OFF position

- connect your FTDI cable to the board

- go int the Tools > Board menu and select "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328 -> optiboot" 

- open the mstore_smash.ino sketch in the Arduino IDE

- upload it on the board

- open a new terminal window in order to follow the work in progress (which should be achieved in
  about 2 minutes)

This process is only required once and won't need to be executed again, unless you replace your 24LC1025
chip with a new one.




D/ ATMEGA328's internal EEPROM writing

Each data logger is identified by an (unique) 8 characters string which must be written in the ATMEGA328's 
EEPROM with the help of an other dedicated sketch.

For this :

- put the GPS's power switch in the OFF position

- connect your FTDI cable to the board

- go int the Tools > Board menu and select "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328 -> optiboot" 

- open the station_id_rec_in_eeprom.ino sketch in the Arduino IDE

- edit the #define STATION_ID "MY_IDENT" directive and set your station ID

- upload the sketch on the board

- open a new terminal window in order to follow the work in progress (which should be achieved in a few seconds)

This process is only required once and won't need to be executed again, unless you need to reset the station's
identifier.




E/ Data logger firmware upload


- put the GPS's power switch in the OFF position

- connect your FTDI cable to the board

- go int the Tools > Board menu and select "Arduino Pro or Pro Mini (3.3V, 8 MHz) w/ ATmega328 -> optiboot" 

- open the gprs_datalogger.ino sketch in the Arduino IDE

- upload it on the board

- disconnect your FTDI cable from the board

- put the GPS's power switch in the ON position (or replug it if it was temporary taken out of the board)






