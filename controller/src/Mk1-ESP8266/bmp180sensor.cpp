#include "config.h"
#include "bmp180sensor.h"



      void Bmp180_sensor::showstate(char *b) {
        sprintf(b,"T=%.1f P=%.1f Pr=%.1f mwd=%d lpt=%d lit=%d need=%d valid=%d state=%d",
        m_LastTemp, m_LastPressure, m_LastRelPressure, 
        m_MillisWhenDone,m_LastPressTime,m_LastPressRqTime,
        m_needcal,m_valid,m_State);
      }

#if 0
      bool Bmp180_sensor::get_last_dpdt(uint8_t distance, double &mbarpersec) {
        
        uint8_t last   = (dstoreidx - 1) & (DSTOREDEPTH-1);
        uint8_t first  = (last - distance) & (DSTOREDEPTH - 1);
        int32_t deltat = datastore[last].mtime - datastore[first].mtime;
        
        if ((last==first) || (distance == 0) || 
            (distance > (DSTOREDEPTH-1)) ||
            (deltat < 1)) {
          return false;
        }
        double deltap = datastore[last].pressure - datastore[first].pressure;
        mbarpersec    = 1000.0 * deltap / deltat;
        return true;
      }
#endif
      int32_t Bmp180_sensor::get_last_dpdt(uint8_t distance, double &mbarpersec) {
        
        uint8_t last   = (dstoreidx - 1) & (DSTOREDEPTH-1);
        uint8_t first  = (last - distance) & (DSTOREDEPTH - 1);
        int32_t deltat = datastore[last].mtime - datastore[first].mtime;
        
        if ((last==first) || (distance == 0) || 
            (distance > (DSTOREDEPTH-1)) ||
            (deltat < 1)) {
          return -1;
        }
        double deltap = datastore[last].pressure - datastore[first].pressure;
        mbarpersec    = 1000.0 * deltap / deltat;
        return deltat;
      }

      int32_t Bmp180_sensor::get_last_dpdt_time(uint8_t distance) {
        
        uint8_t last   = (dstoreidx - 1) & (DSTOREDEPTH-1);
        uint8_t first  = (last - distance) & (DSTOREDEPTH - 1);
        int32_t deltat = datastore[last].mtime - datastore[first].mtime;
        return deltat;
      }
      
      bool Bmp180_sensor::poll(void) {
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
              m_LastRelPressure      = m_LastPressure - m_RefPressure;
              m_LastPressTime        = m_MillisWhenDone;
              uint8_t di             = dstoreidx&(DSTOREDEPTH-1);
              datastore[di].pressure = m_LastPressure;
              datastore[di].mtime    = m_LastPressRqTime;
              dstoreidx++;
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
