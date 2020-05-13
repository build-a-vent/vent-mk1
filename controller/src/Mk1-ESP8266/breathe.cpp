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

#include "breathe.h"
#include "display.h"
#include "bmp180sensor.h"
extern class Bmp180_sensor BreathSensor;


#include "outflow.h"
extern class c_mag_outflow outflow;

#include "pstk.h"


void c_s_breather::handle_monitor(uint8_t index) {
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
      //alert(buffer);
    }
    if ((lastp - cp) < -4) {
      sprintf(buffer,"UDP : %d %d %d", index , cp, lastp);
      //alert(buffer);
    }
  }
}

void c_s_breather::poll(void) {
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
        centertime = now + breathlen / 2;
        state=k_inspir;
      }
      break;
    case k_inspir:
      if ((int32_t)(now-centertime) > 0) {
        deltat_center = BreathSensor.get_last_dpdt(12,dpdt_center);
        centertime += 10000;
      }
      if ((int32_t)(now-cyclestart) > switchiipeip) {
        outflow.set(s_eip);
        handle_monitor(0);
        // FIXME show changes
        state=k_waitair;
      }
      break;
    case k_waitair:
      if ((int32_t)(now-centertime) > 0) {
        deltat_center = BreathSensor.get_last_dpdt(12,dpdt_center);
        centertime += 10000;
      }
      if ((int32_t)(now-cyclestart) > breathlen + BCYCLE_SETUP_MS - 40) {
        outflow.set(s_eip);
        handle_monitor(1);
        deltat_end = BreathSensor.get_last_dpdt(12,dpdt_end);
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
        sprintf (buffer,"%.1f %.1f %d  ",dpdt_center, dpdt_end,deltat_center);
        display.actualize(1,0,buffer);
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

void c_s_breather::stop(void) {
  outflow.set(s_eep2);
  airsource.stop();
  state=k_idle;
}

bool c_s_breather::start(s_param_t t_flow, s_param_t t_plateau, s_param_t t_cycle) {
  if ((t_flow < t_plateau) && (t_cycle > t_plateau)) {
    switchiipeip = 50;
    breathlen    = t_flow;
    switcheipeep = t_plateau;
    cycletime    = t_cycle;
    Serial.println("breathe : running");
    state=k_start;            
    moncycle = 0;
    alertoff();
    return true;
  }
  return false;        
}

void c_s_breather::add_current_status(JsonObject &Obj) {
  Obj.getOrAddMember("a_pip").set(p_imax);
  Obj.getOrAddMember("a_eip").set(p_ei);
  Obj.getOrAddMember("a_eep").set(p_ee);
}

int8_t c_s_breather::command(char *cmd) {
  char buffer[500];      

  if (!strcmp(cmd,"back")) {
    moncycle = 0;
    alertoff();
    return 1;
  }

  if (!strcmp(cmd,"bstop")) {
    stop();
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

c_s_breather breathe; 
