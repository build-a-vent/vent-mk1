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
#include "config.h"
#include "airsource.h"
#include <ArduinoJson.h>

extern c_airsource airsource;
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
      s_param_t centertime; 
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
      double    dpdt_center;
      s_param_t deltat_center;
      double    dpdt_end;
      s_param_t deltat_end;

    public:
    
      c_s_breather() {
        state=k_idle;
        p_imax = p_ei = p_ee = 0.0;
      }

      bool iscritical(void) {
        return (airsource.is_open());
        /* NO time-consuming operations should run during those states */
      }

      void handle_monitor(uint8_t index);

      void poll(void);

      s_param_t getFlowtime(void) { return breathlen; }

      s_param_t getPlateautime(void) { return switcheipeep; }

      s_param_t getCycletime(void) { return cycletime; }

      bool      isRunning(void) { return !(state==k_idle); }

      void      add_current_status(JsonObject &Obj);
      
      int8_t command(char *cmd);

      void stop(void);
      
      bool start(s_param_t t_flow, s_param_t t_plateau, s_param_t t_cycle);



    
      
  };// end class

  
  extern c_s_breather breathe; 



  
#endif VENT_SIMPLE_BREATHER_H
