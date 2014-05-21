/*
 * File : WindRainMeter_DavisSensors.h
 *
 * Version : 1.2
 *
 * Purpose : I2C wind rain meter (Davis sensors) interface library for Arduino
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
 * 1.2 : accumulatedRainfall encoding rule change in method getReport() : / 100.0 instead of / 10.0
 * 
 */



#ifndef WIND_RAIN_METER_DAVIS_SENSORS_h
#define WIND_RAIN_METER_DAVIS_SENSORS_h



#include "Arduino.h"

#include "Wire.h"






#define WIND_RAIN_METER_ADDRESS 0x04



class WindRainMeter {

  public:

    WindRainMeter();
    
    byte reportId;
    
    int elapsedTimeSinceLastReportInSeconds;
    
    float accumulatedRainfallMM;
    
    float currentWindSpeedMetersPerSecond;
    float currentWindDirectionDecimalDegrees;
    
    float meanWindSpeedMetersPerSecond;
    float meanWindDirectionDecimalDegrees;
    
    float maxWindSpeedMetersPerSecond;    
    
    float getReport();
    
  private:

     
};




#endif