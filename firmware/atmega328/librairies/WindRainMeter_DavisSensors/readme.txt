### I2C wind rain meter interface library for Arduino

This library enables the communication, via the I2C protocol, with a MSP430G2553 microcontroller 
acting as a "wind and rain meter / counter" interface with Davis sensors :

- 6410 anemometer with hall effect sensor
- rain collector 2 metric version (7852M)

The Arduino (I2C master) can request "reports" to the MSP430G2553 slave and then obtain data such as :

- the elapsed time since the last report (integer value, in seconds)
- the accumulated rainfall in the rain collector since the last report (float value, in mm)
- the current instantaneous wind speed (float value, in m/s)
- the current instantaneous wind direction (float value, in decimal degrees)
- the mean wind speed since the last report (float value, in m/s)
- the mean wind direction since the last report (float value, in decimal degrees)
- the maximum wind speed since the last report (float value, in m/s)
 
This library currently supports the 0.8.X version of the MSP430G2553 wind and rain meter firmware.   

 
Dependencies : Wire library.


Author : Previmeteo (www.previmeteo.com)

Project web site : http://oses.previmeteo.com/

License: GNU GPL v2 (see License.txt)