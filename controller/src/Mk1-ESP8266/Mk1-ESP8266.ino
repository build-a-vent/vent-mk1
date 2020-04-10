/*******************************************************************
  This file is part of build-a-vent.

    build-a-vent is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    build-a-vent is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with build-a-vent.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

/* build-a-vent Mark one Ventilator (c) 2020 Peter@build-a-vent.org
 *
 *
 * Hardware connections:
 * NodeMCU     BMP180       LCD Display
 * 3.3V         VIN         VCC
 * GND          GND         GND
 *
 * D0           PWM Signal to variable pressure valve
 * D1           SCL         SCL
 * D2           SDA         SDA   
 * D3           keep free, moves during boot and upload
 * D4           keep free, moves during boot and upload - USED here for ALERT LED
 * D6           Switch Signal to Solenoid 1
 *
 *
 *this is INCOMPLETE WORK IN PROGRESS, missing are
 * - eeprom store last settings, auto start with these params after boot
 * - user interface through buttons or wlan
 * - second pressure sensor - or use a relative pressure sensor to
 *   keep track of the external pressure, otherwise wheather changes will
 *   influence your pressure readings
 * - other things
 */







#define PWM_DRIVER_PIN D0
#define AIRSOURCE_PIN D6
#define ALERTLED_PIN D4


#include <Wire.h>
#include <SFE_BMP180.h>   // needs Wire

typedef int32_t s_param_t; // SIGNED!! integer format in numeric & stack operations 

#include "paramstack.h"   // defines and implements the stack object

#include "bmp180sensor.h" // needs SFE_BMP180
SFE_BMP180 BodyPressure;  //
Bmp180_sensor BreathSensor(BodyPressure);

#include "display.h"      // need Wire
#include "stringparser.h" // defines and impl. stringparser, needs stack

#include "airsource.h"
c_airsource airsource(AIRSOURCE_PIN);

//
// outflow controls the variable-pressure valve where exhaled air leaves the system
// max pressure of approx 30 mbar (=cm H2O) during inspiration
// 5..15 mbar during exhalation (PEEP requirements)
//
#include "outflow.h"
c_mag_outflow outflow (PWM_DRIVER_PIN);


// calibrator is able to produce time-controlled flows of air, 
// to calibrate the airflow (in ml/sec)
#include "calibrator.h"   // reqires stack, airsource, BreathSensor;

//
// simple alert lighting a fat red LED ...
//

void alert(char *buf) {
  display.actualize(1,0,buf);
  pinMode(ALERTLED_PIN,OUTPUT);
  digitalWrite(ALERTLED_PIN,0); // LED ON
}

void alertoff(void) {
  display.actualize(1,0,"                   ");
  digitalWrite(ALERTLED_PIN,1); // LED ON
}

//
// the breather automaton 
//

#include "simple_breather.h"
 
double     PZero=(0.0);
double     LastP=(0.0);
bool       PressAvail(false);
int32_t   NowMillis(0);
int32_t   LastPrintTime(0);




void setup()
{
  Serial.begin(115200);
  delay(300);
  Serial.println("\nBOOT: build-a-vent.org Mk1 ESP8266 NodeMCU");

  for (int i=0;i<10;++i) {
    Wire.begin();
    delay(100);
    if (BodyPressure.begin()) {
      Serial.println("BMP180 init success");
      PressAvail=true;
      break;
    } else {
      Serial.println("BMP180 init fail");
      delay(50);
    }
  }
  if (PressAvail) {
  } else {
    Serial.println("cont FAILURE of BreathSensor !!");
  }

  display.initialize();
  NowMillis = LastPrintTime = millis();
}

void BigStatusReading() {
  double Press;
  double RelPress;
  double Temp;
  static uint16_t z = 0;

  BreathSensor.get_last_temp(Temp);
  bool valid = BreathSensor.get_last_pressure(Press);

  char buffer[100];

  if (valid) {
    if (PZero < 100) {
      PZero=Press;
      BreathSensor.zero_relative_pressure();
    }
    BreathSensor.get_last_relative_pressure(RelPress);
    
    Serial.print("pressure: ");
    Serial.print(RelPress,1);
    Serial.print(" mb, ");
    Serial.print(Press,1);
    Serial.print(" mb, ");
    Serial.print(Temp,2);
    Serial.println(" deg C, ");

    BreathSensor.showstate(buffer);
    Serial.println(buffer);

    sprintf(buffer,"%+7.1f %-.1f  ",Press,RelPress);
    display.actualize(0,0,buffer);
  } else {
    Serial.println("invalid reading");;
    display.actualize(0,0,"---invalid---");
  }    
}

//
//
// here comes command interpretation 
// add new object's command methods here
//


uint8_t wplist(char *c, uint8_t len) {
  uint8_t rc;
  if ((rc=stack.command(c)) != 0) return rc;
  if ((rc=airsource.command(c)) != 0) return rc;
  if ((rc=calibrator.command(c)) != 0) return rc;
  if ((rc=outflow.command(c)) != 0) return rc;
  if ((rc=breathe.command(c)) != 0) return rc;
  return 0;
}

void word_process(char *c, unsigned char len) {
  uint8_t rc = wplist(c,len);
  if (rc==-1) {
    Serial.print("ERROR:");
    Serial.println(c);
  }
  if (rc==0) {
    Serial.print("UNKNOWN:");
    Serial.println(c);
  }
}

//
// the loop in arduino programs does nearly everything

void loop()
{

   //
   // feed serial input data to stringparser
   //
   while (Serial.available()) {
     char c = Serial.read();
     if ((c==0x0d)|| (c==0x0a)) {
      Serial.println();
     } else {
       Serial.print(c);    
     }
     stringparser.input(c); 
   }

  //
  // process objects poll methods
  //

  BreathSensor.poll();
  airsource.poll();
  calibrator.poll();
  outflow.poll();
  breathe.poll();

  if (!breathe.iscritical()) {
    // here we might do possibliy time-consuming ops (max 100ms!!)
    int32_t now=millis();
    if ((now - LastPrintTime) > 4000) {
      //
      // just to see some signs of life ...
      //
      BigStatusReading();
      LastPrintTime=now;
      BreathSensor.recalibrate(); // update temp reading
    }  
  }
  
  delay(2); // can even do without

}
