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

  /* 
   * a statemachine for BMP180 sensors. no time is wasted in delay()
   * the sensor reads the temperature ASAP when recalibrate() is called.
   * otherwise, upon calling poll() rescans pressure as fast as possible
   *
   * has some complexity 
   * 
   */
  class Bmp180_sensor {
  
    typedef enum { k_idle = 0, k_awaitT=1, k_awaitP=2, k_err } kState;
  
    private:
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

  
    public:

      void showstate(char *b) {
        sprintf(b,"T=%.1f P=%.1f Pr=%.1f mwd=%d lpt=%d lit=%d need=%d valid=%d state=%d",
        m_LastTemp, m_LastPressure, m_LastRelPressure, m_MillisWhenDone,m_LastPressTime,m_LastPressRqTime,
        m_needcal,m_valid,m_State);
      }
          
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

      bool isvalid(void) {
        return m_valid;
      }

      int32_t get_last_ptime(void) { return m_LastPressTime; }  // time last pressure info was received
      int32_t get_last_itime(void) { return m_LastPressRqTime; }  // time last pressure sense commandd was issued

      bool recalibrate(void)   { m_needcal=true; }
    
      bool poll(void) {
        bool r=false;
        char rc;
        int32_t now=millis();
        switch(m_State) {
          case k_awaitT:
            if ((int32_t)(now - m_MillisWhenDone) > 0) {
              rc = m_pSensor->getTemperature(m_LastTemp);
              if (!rc) {
                m_State=k_err;
                return false;
              }
              m_State=k_idle;
            }
            break;
          case k_awaitP:
            if ((int32_t)(now - m_MillisWhenDone) > 0) {
              rc = m_pSensor->getPressure(m_LastPressure,m_LastTemp);
              if (!rc) {
                m_State=k_err;
                m_valid = false;
                return false;
              }
              r=true;
              m_valid = true;
              m_LastRelPressure = m_LastPressure - m_RefPressure;
              m_LastPressTime   = m_MillisWhenDone;
              m_State=k_idle;
            }
            break;
          case k_idle :
            if (m_needcal) {
              m_needcal=false;
              if (rc = m_pSensor->startTemperature()) {
                m_State=k_awaitT;
                m_MillisWhenDone = now + rc;
              } else {
                m_State = k_err;
                m_valid = false;
              }
            } else {
              if (rc = m_pSensor->startPressure(1)) {
                m_State=k_awaitP;
                m_MillisWhenDone  = now + rc;
                m_LastPressRqTime = now;
              } else {
                m_State = k_err;
                m_valid = false;
              }
            }
            break;
          case k_err:
            rc = m_pSensor->getError();
            Serial.print("Pressure error :");
            Serial.println(rc);
            m_State=k_idle;
            m_valid=false;
          
        } // end case
        return r;
      } // end poll method
  }; // end class

#endif
