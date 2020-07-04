#ifndef VENT_BMP180SENSOR_H
  #define VENT_BMP180SENSOR_H
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
#if SIMULATE_VENT
  #include "sfe_fake_sensor.h"
#else
  #include <SFE_BMP180.h>   // needs Wire
#endif
  /* 
   * a statemachine for BMP180 sensors. no time is wasted in delay()
   * the sensor reads the temperature ASAP when recalibrate() is called.
   * otherwise, upon calling poll() rescans pressure as fast as possible
   *
   * has some complexity 
   * 
   */
  class Bmp180_sensor {

    private:
      typedef enum { k_idle = 0, k_awaitT=1, k_awaitP=2, k_err } kState;

      #define DSTOREDEPTH 16 // must be pwr of 2 !!
      struct s_datastore { // store recent values for dp/dt analysis
        double  pressure;
        int32_t mtime;
      } datastore[DSTOREDEPTH];
    
      double       m_LastTemp;
      double       m_LastPressure;
      double       m_LastRelPressure;
      double       m_RefPressure{1013.25};
      int32_t      m_MillisWhenDone;
      int32_t      m_LastPressTime;
      int32_t      m_LastPressRqTime;
      SFE_BMP180 * m_pSensor;
      bool         m_needcal;
      bool         m_valid;
      kState       m_State;
      uint8_t      dstoreidx;


  
    public:

      void showstate(char *b);
      
          
      Bmp180_sensor (SFE_BMP180 & pSens) {
        m_pSensor     = &pSens;
        m_State       = k_idle;
        m_needcal     = true;
        m_valid       = false;
      }
        
      bool get_last_pressure(double &Pressure) {
        if (m_valid) {
          Pressure=m_LastPressure;
        }
        return m_valid;
      }

      bool get_last_relative_pressure(double &Pressure) {
        if (m_valid) {
          Pressure=m_LastRelPressure;
        }
        return m_valid;
      }

      bool zero_relative_pressure(void) {
        if (m_valid) {
          m_RefPressure = m_LastPressure;
        }
        return m_valid;
      }

      bool get_last_temp(double &Temp) {
        if (m_valid) {
          Temp=m_LastTemp;
        }
        return m_valid;
      }

      int32_t get_last_dpdt(uint8_t distance, double &mbarpersec);

      int32_t get_last_dpdt_time(uint8_t distance);

      bool isvalid(void) { return m_valid; }

      int32_t get_last_ptime(void) { return m_LastPressTime; }  // time last pressure info was received
      
      int32_t get_last_itime(void) { return m_LastPressRqTime; }  // time last pressure sense commandd was issued

      bool recalibrate(void)   { m_needcal=true; }
    
      bool poll(void);
  }; // end class

#endif
