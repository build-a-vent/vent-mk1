#ifndef VENT_PERSIST_H
#define VENT_PERSIST_H
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
#include "configitems.h"
    
  class c_persist {
    #define SLEN 40

    struct saveables {
      s_configblock configblock;
      // configureables are in configblock
      uint32_t cks;
    };

    
    private:
      struct saveables s;
      int32_t lastupdate;
      int32_t lastsaved;
      bool   wasCorrect;
      uint32_t              calcCks(void);      
      static uint32_t       cks_config(uint8_t *p, uint16_t size);
      
    public:
      void                  putSsid(const char * n);
      
      void                  putPwd(const char * n);
      
      bool                  checkCks(void);
      
      inline bool           getLoadCks(void) { return wasCorrect; }
      
      bool                  readFromEeprom(void);
      
      void                  writeToEeprom(void);

      void                  eeflush(void);

      int8_t                command(const char * const);

      inline void           markUpdate(void) { lastupdate = millis(); }

      inline s_configblock* get_configblock(void) { return &(this->s.configblock); }
      
      inline const char *   getSsid(void) { return c_configitems::getStringByName("c_ssid"); }

      inline const char *   getKey(void) { return c_configitems::getStringByName("c_passwd"); }
      
      //inline uint32_t     getCks(void)     { return s.cks; }      
      
      void poll (bool may_write_eeprom);
  };
  
  extern c_persist netconfig;
  
  
#endif // VENT_PERSIST_H
