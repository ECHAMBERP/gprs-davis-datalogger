

// Use with MSP430G22553 @ 1 Mhz


/*
 *
 * Description :
 *
 * Implementation of an I2C slave device acting as an interface with Davis rain gauge and anemometers sensors (tested with refs. 7852M and 6410E)
 *
 * Tested on MSP430G2553 microcontrollers running at 16 Mhz and 1 MHz with ENERGIA IDE :  0101E0010
 *
 * The device address can be specified by the user via the #define DEVICE_ADDRESS directive (see below)
 *
 * The master must request 15 bytes to the slave, which returns as a response a "report" containing the following bytes :   
 *
 * - report id : 1 byte
 * - elapsed time since last report (encoded), in seconds : 2 bytes
 * - accumulated rainfall since last report (encoded), in mm : 2 bytes
 * - current (last tick) wind speed, in meters per second : 2 bytes
 * - current (last tick) wind direction, in decimal degrees : 2 bytes
 * - mean (since last report) wind speed, in meters per second : 2 bytes
 * - mean (since last report) wind direction, in decimal degrees : 2 bytes
 * - maximum (since last report) wind speed, in meters per second : 2 bytes
 *
 *
 * Author : Previmeteo (www.previmeteo.com)
 *
 * Project web site : http://oses.previmeteo.com/
 *
 * License: GNU GPL v2 (see License.txt)
 *
 * Version : 0.8.0
 *
 * Creation date : 2013/12/12
 * 
 */
 



#include <Wire.h>



// device address definition

#define DEVICE_ADDRESS 0x04



// calibration data

#define RAIN_SENSOR_BUCKET_TO_MM 0.2

#define ANEMOMETER_SENSOR_PULSE_PER_SECOND_TO_METERS_PER_SECOND 0.998         



// pins definition
  
#define RAIN_GAUGE_PIN P2_5
  
#define ANEMOMETER_PIN P2_0

#define VANE_POWER_PIN P1_5 
  
#define VANE_ADC_PIN A4         // <-> P1_4



// tick delay definition

#define TICK_DELAY_IN_MS 1000


 




union u_tag {
  
  byte b[2];
  
  byte bval;
  int ival;
  
} U;


byte reportId = 0;

unsigned long lastReportMillis = 0;

unsigned long accumulatedRainSensorPulsesSinceLastReport = 0;   
unsigned long lastRainPulseTimeMicros = 0;

unsigned long accumulatedAnemometerSensorPulsesSinceLastReport = 0;             
unsigned long lastAnemometerPulseTimeMicros = 0;

float lastInstantaneousWindSpeedMetersPerSecond = 0;
float maxWindSpeedMetersPerSecondSinceLastReport = 0;

float lastTickWindSpeedMetersPerSecond = 0;
float lastTickWindDirectionDecimalDegrees = 0;

int cosValuesX1000[19] = {1000, 996, 984, 965, 939, 906, 866, 819, 766, 707, 642, 573, 500, 422, 342, 258, 173, 87, 0};
// cos values X 1000 for : 0°, 5°, 10°, 14°, 20°, 25°, 30°, 35°, 40°, 45°, 50°, 55°, 60°, 65°, 70°, 75°, 80°, 85°, 90°

long accumulatedTicksCosX1000WindDirectionSinceLastReport = 0;
long accumulatedTicksSinX1000WindDirectionSinceLastReport = 0;



    



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// setup() and loop() functions definition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void setup() {
  
  pinMode(RED_LED, OUTPUT); 
  
  pinMode(VANE_POWER_PIN, OUTPUT); 
  digitalWrite(VANE_POWER_PIN, HIGH);       
  
  Wire.begin(DEVICE_ADDRESS);          
  
  Wire.onReceive(twiReceiveEvent); 
  Wire.onRequest(twiRequestEvent); 
  
  pinMode(ANEMOMETER_PIN, INPUT_PULLUP);
  attachInterrupt(ANEMOMETER_PIN, onWindSensorPulse, RISING); 
  
  pinMode(RAIN_GAUGE_PIN, INPUT_PULLUP);
  attachInterrupt(RAIN_GAUGE_PIN, onRainSensorPulse, FALLING); 
  
}

 

void loop() {
  
  onTick();
  
  delay(TICK_DELAY_IN_MS);
  
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// interrupts handlers & helpers definitions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void onRainSensorPulse() {
  
  unsigned long thisRainPulseTimeMicros = micros();
  
  if((thisRainPulseTimeMicros - lastRainPulseTimeMicros) > 500) accumulatedRainSensorPulsesSinceLastReport++;
  
  lastRainPulseTimeMicros = thisRainPulseTimeMicros;

}



void onWindSensorPulse() {
  
  unsigned long thisAnemometerPulseTimeMicros = micros();
  
  unsigned long delayBetweenPulsesInMicros = thisAnemometerPulseTimeMicros - lastAnemometerPulseTimeMicros;
  
  if(delayBetweenPulsesInMicros > 5000) {     
    
    lastInstantaneousWindSpeedMetersPerSecond = 1000000.0 * ANEMOMETER_SENSOR_PULSE_PER_SECOND_TO_METERS_PER_SECOND / delayBetweenPulsesInMicros;
  
    if(lastInstantaneousWindSpeedMetersPerSecond > maxWindSpeedMetersPerSecondSinceLastReport) maxWindSpeedMetersPerSecondSinceLastReport = lastInstantaneousWindSpeedMetersPerSecond;
    
    accumulatedAnemometerSensorPulsesSinceLastReport++;
    
  }
  
  lastAnemometerPulseTimeMicros = thisAnemometerPulseTimeMicros;
  
}



void onTick() {
  
  // wind speed computation
  
  if((micros() - lastAnemometerPulseTimeMicros) > 3000000) lastInstantaneousWindSpeedMetersPerSecond = 0;
  
  lastTickWindSpeedMetersPerSecond = lastInstantaneousWindSpeedMetersPerSecond;
  
    
  // wind direction computation 
  
  long accuVaneADC = 0;

  //digitalWrite(VANE_POWER_PIN, HIGH); 
  
  for(byte i = 0 ; i < 3 ; i++) {
    accuVaneADC += analogRead(VANE_ADC_PIN);
  }
  
  //digitalWrite(VANE_POWER_PIN, LOW); 
  
  lastTickWindDirectionDecimalDegrees = (accuVaneADC * 360) / 3072.0;    // 1024 * 3
    
  accumulatedTicksCosX1000WindDirectionSinceLastReport += getApproxCosDDX1000(lastTickWindDirectionDecimalDegrees);
  accumulatedTicksSinX1000WindDirectionSinceLastReport += getApproxSinDDX1000(lastTickWindDirectionDecimalDegrees);
  
}



float getElapsedTimeSinceLastReportInSeconds() {
  
  float elapsedTimeSinceLastReportInSeconds = (millis() - lastReportMillis) / 1000.0;
  
  return elapsedTimeSinceLastReportInSeconds;
  
}



float getAccumulatedRainfall() {
  
  float accumulatedRainMM = (float) RAIN_SENSOR_BUCKET_TO_MM * accumulatedRainSensorPulsesSinceLastReport;
  
  return accumulatedRainMM;
    
}



float getLastTickWindSpeedMetersPerSecond() {
  
  return lastTickWindSpeedMetersPerSecond;
  
}


float getLastTickWindDirectionDecimalDegrees() {
 
  float lastTickWindDirectionDecimalDegreesReturn = lastTickWindDirectionDecimalDegrees;
  
  if(lastTickWindDirectionDecimalDegreesReturn < 1.0) lastTickWindDirectionDecimalDegreesReturn = 360;
  
  return lastTickWindDirectionDecimalDegreesReturn;
  
}



float getMeanWindSpeedMetersPerSecondSinceLastReport() {
  
  float meanWindSpeedMetersPerSecondSinceLastReport = accumulatedAnemometerSensorPulsesSinceLastReport * ANEMOMETER_SENSOR_PULSE_PER_SECOND_TO_METERS_PER_SECOND / getElapsedTimeSinceLastReportInSeconds();
  
  return meanWindSpeedMetersPerSecondSinceLastReport;
  
}



float getMeanWindDirectionDecimalDegreesSinceLastReport() {
    
//  float windDirectionSectorDecimalDegrees = getWindDirectionSectorDecimalDegrees(radsToDegs(atan2(accumulatedTicksSinX1000WindDirectionSinceLastReport, accumulatedTicksCosX1000WindDirectionSinceLastReport)));
//  
//  if(windDirectionSectorDecimalDegrees < 1.0) windDirectionSectorDecimalDegrees = 360;
  
  float windDirectionSectorDecimalDegrees = radsToDegs(atan2(accumulatedTicksSinX1000WindDirectionSinceLastReport, accumulatedTicksCosX1000WindDirectionSinceLastReport));
  
  if(windDirectionSectorDecimalDegrees < 1.0) windDirectionSectorDecimalDegrees = 360;
  
  
  return windDirectionSectorDecimalDegrees;
  
}




float getMaxWindSpeedMetersPerSecondSinceLastReport() {
  
  return maxWindSpeedMetersPerSecondSinceLastReport;
  
}



void resetCounters() { 
  
  accumulatedAnemometerSensorPulsesSinceLastReport = 0;
  
  accumulatedRainSensorPulsesSinceLastReport = 0;

  maxWindSpeedMetersPerSecondSinceLastReport = 0;
  
  accumulatedTicksCosX1000WindDirectionSinceLastReport = 0;
  accumulatedTicksSinX1000WindDirectionSinceLastReport = 0;

  lastReportMillis = millis();
  
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TWI requests handlers
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void twiReceiveEvent(int howMany) {
  
  if(Wire.available() > 0) {
   
    byte I2CRequestCode = Wire.read();
    
  }

 }

            

void twiRequestEvent() {
 
  // 15 bytes are returned to the master :
  
  // report id : 1 byte
  // elapsed time since last report, in seconds : 2 bytes
  // accumulated rainfall since last report (encoded), in mm : 2 bytes
  // current (last tick) wind speed, in meters per second : 2 bytes
  // current (last tick) wind direction, in decimal degrees : 2 bytes
  // mean (since last report) wind speed, in meters per second : 2 bytes
  // mean (since last report) wind direction, in decimal degrees : 2 bytes
  // maximum (since last report) wind speed, in meters per second : 2 bytes
  
  
  byte respArray[15];
    
  U.bval = reportId;
  
  respArray[0] = U.b[0];
 
  U.ival = (int) getElapsedTimeSinceLastReportInSeconds();
 
  respArray[1] = U.b[0];
  respArray[2] = U.b[1];
  
  U.ival = (int) (getAccumulatedRainfall() * 10);
    
  respArray[3] = U.b[0];
  respArray[4] = U.b[1];
  
  U.ival = (int) (getLastTickWindSpeedMetersPerSecond() * 10);
    
  respArray[5] = U.b[0];
  respArray[6] = U.b[1];
  
  U.ival = (int) (getLastTickWindDirectionDecimalDegrees() * 10);
    
  respArray[7] = U.b[0];
  respArray[8] = U.b[1];
  
  U.ival = (int) (getMeanWindSpeedMetersPerSecondSinceLastReport() * 10);
    
  respArray[9] = U.b[0];
  respArray[10] = U.b[1];
  
  U.ival = (int) (getMeanWindDirectionDecimalDegreesSinceLastReport() * 10);
    
  respArray[11] = U.b[0];
  respArray[12] = U.b[1];
  
  U.ival = (int) (getMaxWindSpeedMetersPerSecondSinceLastReport() * 10);
    
  respArray[13] = U.b[0];
  respArray[14] = U.b[1];
  
  
  Wire.write(respArray, 15); 
  
  resetCounters();
    
  reportId++;
  if(reportId == 255)  reportId = 0;
  
}








/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Mean wind direction computation functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




float getWindDirectionSectorDecimalDegrees(float angleDecimalDegrees) {
  
  // returns a float (multiple of 5°) varying from 0° to 355°
  
  byte windDirectionSectorIndex = getWindDirectionSectorIndex(angleDecimalDegrees);
    
  float windDirectionSectorDecimalDegrees = windDirectionSectorIndex * 5.0;
    
  return windDirectionSectorDecimalDegrees;
  
}



byte getWindDirectionSectorIndex(float angleDecimalDegrees) {

  // returns a byte varying from 0 (0°) to 71 (355°)
  
  byte windDirectionIndex = 0;
  
  long angleDecimalDegreesX10 = angleDecimalDegrees * 10;
    
  if((angleDecimalDegreesX10 <= 25) or (angleDecimalDegreesX10 > 3575)) {
    
    windDirectionIndex = 0;
    
  }

  else {
   
   for(long i = 1 ; i < 72 ; i++) {
       
     if((angleDecimalDegreesX10 > ((50 * i) - 25)) and (angleDecimalDegreesX10 <= ((50 * i) + 25))) {
       
       windDirectionIndex = i;
       break;
              
     }
     
   }
    
    
 }
 
 return windDirectionIndex;
           
}



int getApproxCosDDX1000(float angleDecimalDegrees) {

  int theCosX1000 = 0;
    
  byte windDirectionSectorIndex = getWindDirectionSectorIndex(angleDecimalDegrees);
        
  if ((0 <= windDirectionSectorIndex) and (windDirectionSectorIndex <= 18)) theCosX1000 = cosValuesX1000[windDirectionSectorIndex];
        
  else if((18 <= windDirectionSectorIndex) and (windDirectionSectorIndex <= 36)) theCosX1000 = - cosValuesX1000[(36 - windDirectionSectorIndex)];
    
  else if((36 <= windDirectionSectorIndex) and (windDirectionSectorIndex <= 54)) theCosX1000 = - cosValuesX1000[(windDirectionSectorIndex - 36)];
        
  else if((54 <= windDirectionSectorIndex) and (windDirectionSectorIndex <= 72)) theCosX1000 = cosValuesX1000[(72 - windDirectionSectorIndex)];

  return theCosX1000;

}



int getApproxSinDDX1000(float angleDecimalDegrees) {

  int theSinX1000 = 0;

  if(angleDecimalDegrees < 270) theSinX1000 = - getApproxCosDDX1000(angleDecimalDegrees + 90);
        
  else theSinX1000 = - getApproxCosDDX1000(angleDecimalDegrees - 270);
    
  return theSinX1000;
    
}
    
    

float radsToDegs(float rads) {
  
  float degs = rads * 57.296;
  
  if(degs < 0) degs += 360;
  
  else if(degs > 360) degs -= 360;
  
  return degs;
  
}



      
