/*
 * I2C wind - rain meter interface demonstration
 *
 * The arduino requests peridically reports from a MSP430G2553 microcontroller 
 * acting as a "wind and rain meter / counter" interface with sensors from Davis 
 *
 * - 6410 anemometer with hall effect sensor
 * - rain collector 2 metric version (7852M)
 *
 * Data are reported in mm, m/s and decimal degrees
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
*/



#include <Wire.h>

#include <WindRainMeter_DavisSensors.h>



#define REPORT_PERIOD_IN_SECONDS 300


WindRainMeter windRainMeter;



void setup() {
  
  Serial.begin(9600);       

  Wire.begin();  
  
  Serial.println("Going on...\n");

  delay(1000);
  
}


void loop() {
   
  windRainMeter.getReport();
  
  Serial.print("report counter : ");
  Serial.println(windRainMeter.reportId);
  
  Serial.print("elapsed time since last report : ");
  Serial.print(windRainMeter.elapsedTimeSinceLastReportInSeconds);
  Serial.println(" seconds");
  
  Serial.print("accumulated rainfall since last report : ");
  Serial.print(windRainMeter.accumulatedRainfallMM, 1);
  Serial.println(" mm");
  
  Serial.print("current wind speed : ");
  Serial.print(windRainMeter.currentWindSpeedMetersPerSecond, 1);
  Serial.println(" m/s");
  
  Serial.print("current wind direction : ");
  Serial.print(windRainMeter.currentWindDirectionDecimalDegrees, 0);
  Serial.println(" degrees");
  
  Serial.print("mean wind speed since last report : ");
  Serial.print(windRainMeter.meanWindSpeedMetersPerSecond, 1);
  Serial.println(" m/s");
  
  Serial.print("mean wind direction since last report : ");
  Serial.print(windRainMeter.meanWindDirectionDecimalDegrees, 0);
  Serial.println(" degrees");
  
  Serial.print("max wind speed since last report : ");
  Serial.print(windRainMeter.maxWindSpeedMetersPerSecond, 1);
  Serial.println(" m/s");
  
  Serial.println();
  Serial.println();
  
  delay((long) REPORT_PERIOD_IN_SECONDS * 1000);


}


