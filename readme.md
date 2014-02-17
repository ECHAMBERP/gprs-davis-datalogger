Davis Vantage Integrated Sensor Suite / GPRS datalogger
---

This project aims to develop an autonomous "GPRS datalogger" linked to a Cabled Integrated Sensor Suite (ISS) from Davis, allowing :

- the monitoring and reading of the data provided by the ISS sensors (T / RH sensor, rain collector and anemometer)
- the uploading, at regular time intervals, of the collected data to a web server 

The Sensor Interface Module (SIM) board shipped with the Vantage Pro 2 and it's associated console(s) are thus no longer required for the transmission of the collected data, as you can use the project's board to send this data to the Internet, in a configurable way and to the web server of your choice.

The GPRS communication means that the station can be used far away from a house or any other facility (no more restrictions on the radio range as for the SIMs) and used in an autonomous way. The board has a low power consumption design, which gives it a very good autonomy with Li-Ion batteries recharged by the sun power. It also embeds a GPS module and a RTC clock in order to know exactly where the station is, and what time is it, when it uploads its data.        

Please note that the project's board is currently designed to be used with the latest ISS sensor's generation  :

- temperature & humidity sensor based on the SHT1x sensor from Sensirion
- rain collector II (re)configured for metric measurement
- 6410 anemometer : new version with hall effect sensor

Take a look at the bill of materials for more details on the required modules, ICs and components, for the board's construction.

You will note that the board actually includes 2 microcontrollers, an ATMEGA328 and  a MSP430G2553 (which can be respectively programmed with the ARDUINO and ENERGIA IDEs). The first one embeds the "main intelligence" of the board and the second acts as an "in between" the ATMEGA and the rain and wind sensors, as an I2C slave. The two firmware upload's processes are described in details in the firmware/atmega328/readme.md and firmware/msp430g2553/readme.md files.

The schematics and bill of materials required to assemble the current prototype can be found in the "hardware" directory.

