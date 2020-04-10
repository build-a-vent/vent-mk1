#ifndef VENT_OUTFLOW_H
  #define VENT_OUTFLOW_H

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


  // a variable pressure outflow valve controlled by pwm 
  // the pin number of the pwm is passed as param in constructor.
  // the ESP8266 PWM goes from 0..1023, we use 512..1023
  // the higher the number the lower the current through the coil
  // unfortunately this is highly unlinear, therefore we implemented 
  // a  teachin
  //  -------------- WORK IN PROGRESS ---------------------------

  typedef enum { s_zerop = 0, // no pressure
                 s_iip   = 1, // initial inspiratory press ca 40 mbar
                 s_eip   = 2, // end inspiratory pressure ca 30 mbar
                 s_eep1  = 3, // end expiratory pressure 1 :  5 mbar
                 s_eep2  = 4, // end expiratory pressure 2 : 10 mbar
                 s_eep3  = 5, // end expiratory pressure 3 : 15 mbar
                 s_pressmax = 6
               } kPressSetting;

  class c_mag_outflow {
  
    #define MAG_CALMDOWNTIME 200
    
    // poll drives the SAR setup of the valve
    typedef enum { k_idle = 0, // poll idling
                   k_stabilize = 1,
                   k_read      = 2,
                   k_next      = 3
                 } tState;

    struct pwm2prs {
      s_param_t mbar;
      uint16_t  pwm;
    } p2pbuf[s_pressmax] = {
        {  0,   0 }, // teached in on apr 6 
        { 35, 511 },
        { 30, 267 },
        {  5,  20 },
        { 10,  48 },
        { 15,  60 }        
    };
               
    private:
      int pwmpin;
      tState state{k_idle};
      uint16_t stepwidth;
      uint16_t curval;
      int32_t nexttime;
      uint16_t curpwm;
      kPressSetting cursetting{s_zerop};

      void pwmset (uint16_t pwm) {
        pwm &= 0x1ff; //0..511
        analogWrite(pwmpin,1023 - pwm);          
      }

  
    public:
      c_mag_outflow (int pinno) {
        pwmpin = pinno;
        cursetting = s_zerop;
        pwmset(0);
      }

      void set (uint8_t index) {
        if (index >= s_pressmax) {
          index = 0;
        }
        cursetting=(kPressSetting)index;
        pwmset(p2pbuf[cursetting].pwm);
      }
  
      void putconfig (char *buffer) { 
        uint16_t offset = 0;
        uint8_t i=0;
        for (; i< s_pressmax; ++i) {
          offset += sprintf(buffer+offset,
            "%d %d %d outfcfg\n",i,p2pbuf[i].mbar,p2pbuf[i].pwm);
          sprintf(buffer+offset,"%d outfset\n",cursetting);        
        }
      }
      
      void show(char *buffer) {
        sprintf(buffer,"idx=%d, mbar=%d pwm=%d",cursetting,p2pbuf[cursetting].mbar,p2pbuf[cursetting].pwm);
        
      }

      void poll(void) {
        int32_t now = millis();
        switch (state) {
          case k_idle : 
            break;
            // this implements a successive approximation
          case k_stabilize:
            if ((int32_t)(BreathSensor.get_last_itime() - nexttime)>0) { 
              // we stabilized  AND new pressure read cmd was issued
              nexttime=now;
              state=k_read;
            }          
            break;
          case k_read:
            if ((int32_t)(BreathSensor.get_last_ptime() - nexttime)>0) { 
              double pcur;
              BreathSensor.get_last_relative_pressure(pcur);
              s_param_t ipress = pcur;
              if (pcur > p2pbuf[cursetting].mbar) {
                curval -= stepwidth;
              } else {
                curval += stepwidth;
              }
              stepwidth >>=1; // shift 1 bit dn
              state=k_next;
            }
            break;
          case k_next:
            if (stepwidth) {
              nexttime = now + MAG_CALMDOWNTIME;
              state    = k_stabilize;
              pwmset(curval);
            } else {
              p2pbuf[cursetting].pwm = curval;
              state = k_idle;
              airsource.stop();
            }
            break;
          default:
            Serial.print("outflow: unhandled state : ");
            Serial.println(state);
            state = k_idle;
        }        
      }

      int8_t command(char *cmd) {
        static char buffer[1000];      
  
        if (!strcmp(cmd,"outfcfg")) {  // TopOfStack : mbar pwm idx
          s_param_t n_pwm  = stack.spop();
          s_param_t n_mbar = stack.spop();
          s_param_t idx    = stack.spop();
          if ((idx < 0) || (idx >= s_pressmax)) {
            Serial.println("outfcfg : wrong idx / not enough data");
            return -1;
          } else {
            p2pbuf[(kPressSetting)idx].mbar=n_mbar;
            p2pbuf[(kPressSetting)idx].pwm=n_pwm;
            return 1;
          }
        }
  
        if (!strcmp(cmd,"outfset")) {  // TopOfStack :  idx
          s_param_t idx  = stack.spop();
          if ((idx < 0) || (idx >= s_pressmax)) {
            Serial.println("outfset : wrong idx");
            return -1;
          } else {
            set(idx);
            return 1;
          }
        }
        if (!strcmp(cmd,"outfstat")) { 
          show(buffer);
          Serial.println(buffer);
          return 1;
        }
        if (!strcmp(cmd,"outfdump")) { 
          putconfig(buffer);
          Serial.println(buffer);
          return 1;
        }
        if (!strcmp(cmd,"outfteach")) {  // TopOfStack : mbar idx
             s_param_t time  = stack.spop();
             if (time>0) airsource.start(time);
             stepwidth = 128;
             curval    = 256;
             state     = k_next;
             return 1;
        }
        return 0; // not my business
      }
};  // end class

  
#endif // VENT_OUTFLOW_H
