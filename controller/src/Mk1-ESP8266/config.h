#ifndef VENT_CONFIG_H
#define VENT_CONFIG_H
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

  #define SIMULATE_VENT              1 // set to 1 to create a vent simulator without sensors or actors
  #define HAS_WEBSERVER              0 // set to 1 to add webserver functionality
  #define PERIODIC_BCAST             0 // set to the no of milliseconds between bcasts, 0 for no bcasts
  #define HAS_DISPLAY                0
  #define USE_SYNONYMES              1 // allow json synonymes
  #define LASTUPDATE_TO_WRITE_MS 30000 // 30 sec after last update EEPROM will be written
  #define LOGALOTMORE                1 // log stuff to usb serial
  #define IPADDRESS_FOLLOWS_MAC      1 // 12 bits of mac are in IP
  
  #include <stdint.h>
  typedef int32_t s_param_t; // SIGNED!! integer format in numeric & stack operations 
  #define LOCAL_UDP_PORT 1111
  
  #if HAS_WEBSERVER
    #define LOCAL_HTTP_PORT 80
  #endif

// IO PINS 

  #define PWM_DRIVER_PIN       D0//GPIO16
  #define I2C_SDA_PIN          D1//GPIO5
  #define I2C_SCK_PIN          D2//GPIO4
  #define ONEWIRE_T_BTL_PIN    D6//GPIO12
  #define ONEWIRE_T_AIR_PIN    D7//GPIO13
  #define AIRSOURCE_PIN        D8//GPIO15
  #define ALERTLED_PIN         D4//GPIO2
  #define HEATER_PIN           D3//GPIO0


#endif VENT_CONFIG_H
