#ifndef VENT_SIMPLE_BREATHER_H
  #define VENT_SIMPLE_BREATHER_H

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

  //  -------------- WORK IN PROGRESS ---------------------------


  class ivg8 { // integer averager 8
      s_param_t accu{0};
    
    public:

      void addval(s_param_t value) {
        accu += value - accu/8;
      }
      void initval(s_param_t value) {
        accu += value - accu/8;
      }
      s_param_t getval(void) {
        return accu/8;
      }
  };
    
  // a simple breather controller with monitor for 
  // - peak inspir. pressure PIP
  // - end inspir. pressure  EIP
  // - plateau pressure
  // - end expir. pressure

  // needs stack, airsource, BreathSensor, outflow, display


  #define BCYCLE_SETUP_MS 100  // time btw valve energizing to airflo start
  
  class c_s_breather {
    //
    // again the states the breather engine can be in, as 
    // in all other classes with a poll method
    //
    typedef enum {
      k_idle    = 0,
      k_prepare = 1,
      k_inspir  = 2,
      k_waitair = 3,
      k_plateau = 4,
      k_exspir  = 5,
      k_start   = 6,
      k_sync    = 7,      
    } t_br_stat;

    
    private:
      s_param_t switchiipeip;  // when to switch from iip to eip
      s_param_t breathlen;     // how long does air flow
      s_param_t switcheipeep;  // end of plateau phase, sw to eep;
      s_param_t cycletime;      
      s_param_t moncycle;
      s_param_t p_imax;        // max inspir. press
      s_param_t p_ei;          // end inspir. press
      s_param_t p_ee;          // end exspir. press
      int32_t   cyclestart;
      int32_t   ptstore;       // store last press meas time.
      s_param_t lastp;
      ivg8      pressmon[4];
      char      buffer [100];
      t_br_stat state{k_idle};

    public:
      c_s_breather() {
        state=k_idle;
        p_imax = p_ei = p_ee = 0.0;
      }


      bool iscritical(void) {
        return (airsource.is_open());
        /* NO time-consuming operations should run during those states */
      }

      void handle_monitor(uint8_t index) {
        if (moncycle==3) {
          pressmon[index].initval(lastp);
        }
        if (moncycle < 21) {
          pressmon[index].addval(lastp);          
        }
        if (moncycle > 20) {
          s_param_t cp = pressmon[index].getval();
          if ((lastp - cp) > 5)  {
            sprintf(buffer,"OVP : %d %d %d", index , cp, lastp);
            alert(buffer);
          }
          if ((lastp - cp) < -4) {
            sprintf(buffer,"UDP : %d %d %d", index , cp, lastp);
            alert(buffer);
          }
        }
      }

      void poll(void) {
        int32_t now = millis();
        static char buffer[100];
        int32_t newt = BreathSensor.get_last_ptime();        
        if (newt != ptstore) { // new meas
          double z;
          BreathSensor.get_last_relative_pressure(z);
          lastp = z;
          if (lastp > p_imax) p_imax = lastp;
        }

        switch (state) {
          case k_idle : 
            break;
          case k_prepare:
            if ((int32_t)(now-cyclestart) > BCYCLE_SETUP_MS) {
              airsource.start(breathlen);
              state=k_inspir;
            }
            break;
          case k_inspir:
            if ((int32_t)(now-cyclestart) > switchiipeip) {
              outflow.set(s_eip);
              handle_monitor(0);
              // FIXME show changes
              state=k_waitair;
            }
            break;
          case k_waitair:
            if ((int32_t)(now-cyclestart) > breathlen) {
              outflow.set(s_eip);
              handle_monitor(1);
              // FIXME show changes
              state=k_plateau;
            }
            break;
          case k_plateau:
            if ((int32_t)(now-cyclestart) > switcheipeep) {
              handle_monitor(2);
              p_ei = lastp;
              outflow.set(s_eep2);
              // FIXME show changes
              state=k_exspir;
            }
            break;
          case k_exspir :
            if ((int32_t)(now - cyclestart) > cycletime) {
              handle_monitor(3);
              p_ee = lastp;
              sprintf (buffer,"PI=%+3d EI=%+3d EE%+3d",p_imax, p_ei, p_ee);
              display.actualize(2,0,buffer);
              sprintf (buffer,"I%4d P%4d C%4d",breathlen, switcheipeep, cycletime);
              display.actualize(3,0,buffer);
              outflow.set(s_iip); // limit to high iip;
              p_imax = 0;
              cyclestart = now;
              ++moncycle;
              state = k_prepare;           
            }
            break;
          case k_start:
            moncycle = 0;
            airsource.stop();
            cyclestart = now - cycletime + 2000; // 2 sec exspiration to start with,
            outflow.set(s_eep2);
            p_imax = p_ee = 0.0;
            state = k_exspir;
            break;
          default:
            Serial.print("outflow: unhandled state : ");
            Serial.println(state);
            state = k_idle;
        }        
      }

      int8_t command(char *cmd) {
        static char buffer[1000];      
  
        if (!strcmp(cmd,"back")) {
          moncycle = 0;
          alertoff();
          return 1;
        }

        if (!strcmp(cmd,"bstop")) {
          outflow.set(s_eep2);
          airsource.stop();
          state=k_idle;
          return 1;
        }

        if (!strcmp(cmd,"badump")) {
          for (uint8_t i=0;i<4;++i) {
            sprintf(buffer,"ALR %d = %d",i,pressmon[i].getval());
            Serial.println(buffer);
          }
          return 1;
        }

        if (!strcmp(cmd,"breathe")) {

          s_param_t t_cyc  = stack.spop();
          s_param_t t_plat = stack.spop();
          s_param_t t_insp = stack.spop();
          s_param_t t_topp = stack.spop();
          if (t_topp < 50) {
            Serial.println("t_topp to small");
            return -1;
          }
          if (t_insp < (t_topp+100)) {
            Serial.println("t_insp to small");
            return -1;
          }
          if (t_plat < (t_insp+100)) {
            Serial.println("t_plat to small");
            return -1;
          }
          if (t_cyc < (t_plat+500)) {
            Serial.println("t_cyc : exspir too short");
            return -1;
          }
          switchiipeip = t_topp;
          breathlen    = t_insp;
          switcheipeep = t_plat;
          cycletime    = t_cyc;
          Serial.println("breathe : running");
          state=k_start;            
          moncycle = 0;
          alertoff();
          return 1;
        }
        return 0;
      }  

    
      
  } breathe; // end class




  
#endif VENT_SIMPLE_BREATHER_H
