
The GPRS / Davis sensors logger board includes 2 microcontrollers :


1/ an ATMEGA328 : the "main intelligence" part of the station sits in this microcontroller which accesses 
  and controls directly the real time clock, the T / RH / P sensors, the GPS module and the GPRS modem. 
  
  The ATMEGA runs periodically tasks such as :
  
  - read the sensors values (including the "wind and rain meter" described below)
  - acquire the global position of the station
  - build and store reports, and send them to the web server
  
  The microcontroller is powered by 3.3V and runs @ 8Mhz. It is put in sleep mode between tasks, which 
  reduces substantially it's global power consumption.
  
  The firmware is developed and can be uploaded with the ARDUINO IDE and an FTDI cable.
  
  
2/ a MSP430G2553 : this microcontroller act as an "in between" the ATMEGA328 and the Davis wind and rain
  sensors.  Designed as an I2C slave, it continuously monitors these two sensors and returns "reports"
  to the ATMEGA328 (the I2C master) on request. The reports contains data such as the elapsed time and
  accumulated rainfall since the last report, the current and mean wind speed and direction...
 
  The MSP430G2553 is powered by 3.3V and runs @ 1 Mhz. The main advantage of using it as this frequency 
  is that it's supply's current remains then low in active mode, below 0.3 mA.
  
  The MSP430G2553's firmware is developed and can be uploaded with the help of the ENERGIA IDE and a
  LAUNCHPAD MSP-EXP430G2 board from Texas Instruments.
  
  For more details on the interaction between the ATMEGA328 and the MSP430G2553, please take a look
  at the Arduino's WindRainMeter_DavisSensors library example file.
  
  
The firmware upload's processes of the two microcontrollers are described in details in the 
atmega328/readme.md and msp430g2553/readme.md files.
  
  
  
  
  
  