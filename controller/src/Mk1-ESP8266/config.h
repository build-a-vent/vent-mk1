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
  #include <stdint.h>
  typedef int32_t s_param_t; // SIGNED!! integer format in numeric & stack operations 
  #define LocalUdpPort 1111
  #define LOCAL_HTTP_PORT 80


  #define PWM_DRIVER_PIN D0
  #define AIRSOURCE_PIN D6
  #define ALERTLED_PIN D4


  void alert(char *buf);
  void alertoff(void);




#endif VENT_CONFIG_H
