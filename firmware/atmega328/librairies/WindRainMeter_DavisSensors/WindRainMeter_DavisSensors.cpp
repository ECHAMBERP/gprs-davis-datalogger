/*
 * File : WindRainMeter_DavisSensors.cpp
 *
 * Version : 0.8.0
 *
 * Purpose : I2C wind rain meter (Davis sensors) interface library for Arduino
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
 * Creation date : 2014/01/31
 * 
 */
 
 
 
 
 #include "Arduino.h"

#include "WindRainMeter_DavisSensors.h"






union u_tag {
  
  byte b[2];
  
  byte bval;
  int ival;
  
} U;





 

WindRainMeter::WindRainMeter() { 
  
  Wire.begin();
  
  reportId = 0;
  
  elapsedTimeSinceLastReportInSeconds = 0;
  
  accumulatedRainfallMM = 0;
  
  currentWindSpeedMetersPerSecond = 0;
  currentWindDirectionDecimalDegrees = 0;
    
  meanWindSpeedMetersPerSecond = 0;
  meanWindDirectionDecimalDegrees = 0;
    
  maxWindSpeedMetersPerSecond = 0;   
  
}



float WindRainMeter::getReport() { 
  
  byte reportBytes[15];
  
  Wire.beginTransmission(WIND_RAIN_METER_ADDRESS);    
  
  Wire.requestFrom(WIND_RAIN_METER_ADDRESS, 15);    
  
  for(byte i = 0 ; i < 15 ; i++) {
    reportBytes[i] = Wire.read(); 
  }

  Wire.endTransmission(); 
  
  U.b[0] = reportBytes[0];
  U.b[1] = 0;
  
  reportId = U.bval;
  
  U.b[0] = reportBytes[1];
  U.b[1] = reportBytes[2];
  
  elapsedTimeSinceLastReportInSeconds = U.ival;  
  
  U.b[0] = reportBytes[3];
  U.b[1] = reportBytes[4];
  
  accumulatedRainfallMM = (float)(U.ival / 10.0);
  
  U.b[0] = reportBytes[5];
  U.b[1] = reportBytes[6];
  
  currentWindSpeedMetersPerSecond = (float)(U.ival / 10.0);
  
  U.b[0] = reportBytes[7];
  U.b[1] = reportBytes[8];
  
  currentWindDirectionDecimalDegrees = (float)(U.ival / 10.0);
  
  U.b[0] = reportBytes[9];
  U.b[1] = reportBytes[10];
  
  meanWindSpeedMetersPerSecond = (float)(U.ival / 10.0);
  
  U.b[0] = reportBytes[11];
  U.b[1] = reportBytes[12];
  
  meanWindDirectionDecimalDegrees = (float)(U.ival / 10.0);
  
  U.b[0] = reportBytes[13];
  U.b[1] = reportBytes[14];  
  
  maxWindSpeedMetersPerSecond = (float)(U.ival / 10.0);

}



