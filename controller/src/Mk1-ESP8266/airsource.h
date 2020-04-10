#ifndef VENT_AIRSOURCE_H
  #define VENT_AIRSOURCE_H
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

/* this file defines the object for the air source which supplies
 * breathing air to the ventilation tube */


class c_airsource {
  private:
    int  valvepin;
    bool isactive   {false};
    bool valveopen  {false};
    bool sensitive  {false};
    int32_t activation_time;
    int32_t opening_time;
    int32_t total_millis;
    int32_t remain_millis;
    s_param_t press_idx_act;
    s_param_t press_idx_top;

    inline void setvalve(bool t) {
      digitalWrite(valvepin,t?1:0);      
      valveopen=t;
    }
    void ctrlvalve(bool t, int32_t now) {
      if (t && (!valveopen)) {
        opening_time = now;
      }
      if ((!t) && valveopen) {
        remain_millis -= (now - opening_time);
      }
      setvalve(t);
    }
    
  public:
    c_airsource(int vpin) {  // constructor, here the pin to use is defined
      pinMode(vpin,OUTPUT);
      setvalve(false);
      digitalWrite(valvepin,0);
      valvepin = vpin;
      sensitive = false;
      isactive  = false;
    }

    bool is_active(void) { return isactive; }
    
    bool is_open(void)   { return valveopen; }

    void start(int32_t duration_ms) { // open valve for a specific time in milliseconds
      int32_t now     = millis();
      activation_time = now;
      isactive        = true;
      total_millis    = 
      remain_millis   = duration_ms;
      //
      // we didnt check for valve state
      // we might consider to open only on inactive
      //
      valveopen       = false; 
      ctrlvalve(true,now);
    }

    void stop(void) {
      int32_t now     = millis();
      ctrlvalve(false,now);
      isactive        = false;
    }

    void poll(void) {
      int32_t now     = millis();
      if (isactive) {
        if (sensitive) {
          
        } else {
          if (valveopen) {
            if (remain_millis < (now - opening_time)) {
              ctrlvalve(false,now);
              isactive        = false;
            }
          } else {
            if (remain_millis > 0) {
              ctrlvalve(true,now);
            }
          }
        }
      }
    } // end poll

    int8_t command(char *cmd) {
      if (!strcmp(cmd,"air")) { 
        s_param_t duration = stack.spop();
        if ((duration > 0) && (duration < 15000)) {
          start(duration);
          return 1;
        }
        return -1;
      }

      if (!strcmp(cmd,"astop")) { 
        stop(); 
        return 1; 
      }

      if (!strcmp(cmd,"avalv")) { 
        s_param_t x = stack.spop();
        setvalve(x==1);
        return 1; 
      }

      if (!strcmp(cmd,"ashow")) { 
        char buffer[100];
        sprintf(buffer,"air : %c %c %d %d",isactive?'Y':'N',valveopen?'Y':'N',total_millis,remain_millis);
        Serial.println(buffer);
        return 1; 
      }
      
      return 0;
      
    }
  
};


  
#endif // VENT_AIRSOURCE_H
