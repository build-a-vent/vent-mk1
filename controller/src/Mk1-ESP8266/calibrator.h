#ifndef VENT_CALIBRATOR_H
  #define VENT_CALIBRATOR_H

// this calibrator needs to know the airsource object, the stack, the bmp180sensor and the Serial 
// plus the outflow object to calibrate that


  class c_calibrator {
  
    private:
    typedef enum { k_idle         = 0, 
                   k_await_press1 = 1, 
                   k_await_aircpl = 2, 
                   k_await_calm   = 3, 
                   k_await_pi2    = 4, 
                   k_await_press2 = 5,
                   k_await_sec1   = 6,
                   k_await_sec2   = 7,
                   k_await_sec3   = 8,
                 } kState;
                   
      kState state{k_idle};
      int32_t storedtime;
      s_param_t duration;
      double p1,p2;
      

      void statusprint(int32_t t) {
        static char buffer[100];
        sprintf (buffer,"calib: %d ms : %.1lf %.1lf",t,p1,p2-p1);
        Serial.println(buffer);
      }
      
    public:
      void poll(void) {
        int32_t now = millis();
        switch (state) {
          case k_idle : 
            break;
          case k_await_press1:
            if (BreathSensor.isvalid()) {
              if ((int32_t)(BreathSensor.get_last_ptime() - storedtime) > 0) {
                BreathSensor.get_last_pressure(p1);
                storedtime=now;
                airsource.start(duration);
                state=k_await_aircpl;
              } // else keep waiting
            } else {
              Serial.println("no valid pressures");
              state = k_idle;
            }
            break;
          case k_await_aircpl :
            if (!airsource.is_active()) {
              storedtime  = now + 100; // calm air for 20 ms
              state = k_await_calm;
            } 
            break;
          case k_await_calm :
            if ((int32_t)(now-storedtime) > 0) {
              storedtime  = now;
              state = k_await_pi2;
            } 
            break;
          case k_await_pi2 :
              if ((int32_t)(BreathSensor.get_last_itime() - storedtime) > 0) {
                storedtime=now;
                state=k_await_press2;
              } // else keep waiting
            break;
          case k_await_press2:
            if ((int32_t)(BreathSensor.get_last_ptime() - storedtime) >= 0) {
              BreathSensor.get_last_pressure(p2);
              statusprint(duration);
              storedtime=now+1000;
              state=k_await_sec1;
            } // else keep waiting
            break;
          case k_await_sec1:
            if ((int32_t)(BreathSensor.get_last_ptime() - storedtime) >= 0) {
              BreathSensor.get_last_pressure(p2);
              statusprint(1000);
              storedtime=now+1000;
              state=k_await_sec2;
            } // else keep waiting            
            break;
          case k_await_sec2:
            if ((int32_t)(BreathSensor.get_last_ptime() - storedtime) >= 0) {
              BreathSensor.get_last_pressure(p2);
              statusprint(1000);
              storedtime=now+1000;
              state=k_await_sec3;
            } // else keep waiting            
            break;
          case k_await_sec3:
            if ((int32_t)(BreathSensor.get_last_ptime() - storedtime) >= 0) {
              BreathSensor.get_last_pressure(p2);
              statusprint(1000);
              state=k_idle;
            } // else keep waiting            
            break;
          default:
            Serial.print("calibrator: unhandled state : ");
            Serial.println(state);
            state = k_idle;

        }
        
      }

      int8_t command(char *cmd) {

        if (!strcmp(cmd,"flowcal")) { 
          // start a flow calibration cycle
          duration=stack.spop();
          if (duration < 0) {
            Serial.println("too little time");
            return 1;
          }
          if (duration > 2000) {
            Serial.println("too much time");          
            return 1;
          }
          if (state!=k_idle) {
            char buffer[100];
            sprintf (buffer,"sta:%d time:%d dura:%d delta:%d",state,storedtime,duration,(int32_t)(BreathSensor.get_last_ptime() - storedtime));
            Serial.println(buffer);          
            return 1;
          }
          storedtime = millis();
          state = k_await_press1;
          return 1;        
        }
        return 0; // not my business
      }



  
  } calibrator; // end class and implementation;
  









  
#endif //VENT_CALIBRATOR_H
