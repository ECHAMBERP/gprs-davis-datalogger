
### MSP430G2553 programming


The MSP430G2553 which sits on the board can NOT (at this time) be programmed "in situ" and thus requires the use
of an external programmer.

The simplest and cheapest programmer usable with the ENERGIA IDE is the LAUNCHPAD MSP-EXP430G2, rev 1.5, so we
will use this board to upload the firmware on a MSP430G2553, before extracting it and replugging it later on the 
datalogger's board.

There are several steps required to setup the ENERGIA IDE and upload the firmware on the MSP430G2553. 

Here they are :



A/ IDE and drivers setup 

- download and install the ENERGIA IDE from http://arduino.cc/en/main/software (tested version : 0101E0010)

- download and install the Launchpad USB drivers corresponding to your operating system
  (http://energia.nu/Guide_index.html)




B/ Creation of new "MSP430G2553 boards" in the ENERGIA IDE

The MSP430G2553 microcontroller can be configured to be used at several clock frequencies : 16 MHz, 8 MHz and
1 Mhz. 

The main advantage of using the MSP430G2553 @ 1 Mhz (when of course the final destination and use of the 
microcontroller are compatible with this clock speed…) is that it's active mode supply's current becomes 
then as small as 0.3 mA @ 3.3 V, allowing it to be used in several low power consumption applications.

By default, the ENERGIA IDE only exposes the MSP430G2553 running @ 16 MHz, so we need to add new configuration
data if we want to use the microcontroller @ 8 or 1 MHz.

This can be done as follow :

- open the hardware/msp430/boards.txt file in your text editor

- add, at the end of the file, the following lines :

##############################################################

lpmsp430g2553mhz8.name=LaunchPad w/ msp430g2553 (8MHz)
lpmsp430g2553mhz8.upload.protocol=rf2500
lpmsp430g2553mhz8.upload.maximum_size=16384
lpmsp430g2553mhz8.build.mcu=msp430g2553
lpmsp430g2553mhz8.build.f_cpu=8000000L
lpmsp430g2553mhz8.build.core=msp430
lpmsp430g2553mhz8.build.variant=launchpad

##############################################################

lpmsp430g2553mhz1.name=LaunchPad w/ msp430g2553 (1MHz)
lpmsp430g2553mhz1.upload.protocol=rf2500
lpmsp430g2553mhz1.upload.maximum_size=16384
lpmsp430g2553mhz1.build.mcu=msp430g2553
lpmsp430g2553mhz1.build.f_cpu=1000000L
lpmsp430g2553mhz1.build.core=msp430
lpmsp430g2553mhz1.build.variant=launchpad

##############################################################

- save the file and relaunch the ENERGIA IDE

You should now see two new "boards" in the Tools > Board menu :

-> "LaunchPad w/ msp430g2553 (8MHz)"

-> "LaunchPad w/ msp430g2553 (1MHz)"




C/ Firmware upload

- plug the Launchpad board to your computer 

- launch the ENERGIA IDE

- select the "LaunchPad w/ msp430g2553 (1MHz)" board in the Tools > Board menu 

- open the davis_wind_rain_meter.ino sketch and upload it on the board

Once the upload is achieved, don't forget to unplug the Launchpad from your computer before extracting the 
MSP430G2553 from it's socket.




D/ Insertion of the MSP430G2553 on the datalogger's board



Reminder : the MSP430G2553 is programmed to act as an I2C slave device, and thus can be used as an Arduino 
peripheral device. An Arduino library has been created to facilitate communication between those two,
it can be found in the firmware/atmega328/librairies/WindRainMeter_DavisSensors directory.  

