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
#include "persist.h"
#include <EEPROM.h>
#include <stdint.h>

uint32_t c_persist::cks_config(uint8_t *p, uint16_t size) {
  uint32_t accu=0;
  for (int i = 0;i<size;++i) {
    uint8_t ovf = (accu &0x80000000)?1:0;
    accu = (accu<<1)^p[i]^ovf;
  }
  return accu;
}
  
void c_persist::putSsid(const char * n) {
  s.ssid[SLEN-1]=0;
  strncpy(s.ssid,n,SLEN-1);
}      

void c_persist::putKey(const char * n) {
  s.pwd[SLEN-1]=0;
  strncpy(s.pwd,n,SLEN-1);
}      
 
uint32_t c_persist::calcCks(void) {
  s.cks = 0;
  uint32_t n = cks_config((uint8_t *)&(this->s),sizeof(struct saveables));
  s.cks = n;
  return n;
}

bool c_persist::checkCks(void) {
  uint32_t t=s.cks;
  uint32_t n = calcCks();
  s.cks=t;
  return (n==t);
}

bool c_persist::readFromEeprom(void) {
  uint8_t *wp = (uint8_t *)&(this->s);
  EEPROM.begin(4095);
  for (int i=0;i<sizeof(struct saveables);++i)
  wp[i]=EEPROM.read(i);
  EEPROM.end();
  this->wasCorrect = checkCks();
  return wasCorrect;
  lastupdate = lastsaved = millis(); // nothing dirty
}

void c_persist::writeToEeprom(void) {
  //Serial.printf("EEPROM write size %d addr 0x%x ssid=\"%s\", key=\"%s\"\n",
  //sizeof(*this),LOCATION_NETCONFIG,getSsid(),getKey());
  uint8_t *wp = (uint8_t *)&(this->s);
  calcCks();
  EEPROM.begin(4095);
  for (int i=0;i<sizeof(struct saveables);++i)
  EEPROM.write(i,wp[i]);
  EEPROM.commit();
  EEPROM.end();
  lastsaved = lastupdate; // not dirty
}

c_persist netconfig;

  
