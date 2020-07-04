#ifndef VENT_ALERT_H
  #define VENT_ALERT_H
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
  
  #include "config.h"
  
  
  class c_alert {
    public:
 
      typedef enum { k_overpress_inspir   = 0, 
                     k_underpress_inspir  = 1, 
                     k_overpress_exspir   = 2,
                     k_underpress_exspir  = 3,
                     k_num_alert_reasons  = 9  // MUST BE HIGHEST AND LAST
                   } kAlertReason;
    private:
      bool    status[k_num_alert_reasons];
      bool    islight;
      int32_t nxstep;
      
      bool anytrue(void) {
        for (uint8_t z = 0; z < k_num_alert_reasons; ++z) {
          if (status[z]) {
            return true;
          }
        }
        return false;
      }
      
    public:
    
      void set (kAlertReason which, bool how) {
        status[which] = how;
      }
      
      c_alert() {
        for (int z = 0; z < k_num_alert_reasons; ++z) status[z]=false;
        nxstep = millis(); 
        islight = false;
      }

      void poll(void) {
        if ((millis() - nxstep) > 0) {
          nxstep += 300;
          if (islight) {
            islight=false;
            digitalWrite(ALERTLED_PIN,HIGH);
          } else {
            if (anytrue()) {
              pinMode(ALERTLED_PIN,OUTPUT);
              digitalWrite(ALERTLED_PIN,HIGH);
              islight=true;
            }
          }
        }
      }

      
      bool get (kAlertReason which) {
        return status[which];
      }
    
  }; // end class
#endif //VENT_ALERT_H
