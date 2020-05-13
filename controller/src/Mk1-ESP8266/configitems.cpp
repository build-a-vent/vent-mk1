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
#include <stdint.h>
#include "configitems.h"
#include "config.h"
#include "persist.h"
#include <ArduinoJson.h>


#define CFGDESC_M(EXP) #EXP, CFGOFFS(EXP)

static struct s_configdesc configdesc[] = {
      { CFGDESC_M(c_pip),          20, 60, true },
      { CFGDESC_M(c_lip),        -10,  40, true },
      { CFGDESC_M(c_pep),          5,  30, true },
      { CFGDESC_M(c_lep),         -5,  30, true },
      { CFGDESC_M(c_flair),     400, 1500, true },
      { CFGDESC_M(c_flo2),      400, 1500, true },
      { CFGDESC_M(c_airt),        0, 1300, true },
      { CFGDESC_M(c_o2t),         0, 1300, true },
      { CFGDESC_M(c_inspt),     600, 2600, true },
      { CFGDESC_M(c_cyclt),    2000, 4000, true },
      { CFGDESC_M(c_wtemp),      20,   44, true },
      { CFGDESC_M(c_name),        1,   15, false},
      { NULL,sizeof(s_configblock),1,  15, false}      
  };

struct s_configdesc * c_configitems::get_cfgdesc_by_name(const char * const pname) {
  for (uint8_t i=0; configdesc[i].name!=NULL;++i) {
    if (!strcmp(pname,configdesc[i].name)) {
      return &configdesc[i];
    }
  }
  return NULL; // not found
}

uint8_t c_configitems::serialize_config(JsonObject &Obj) {
  uint8_t* pcfgb = (uint8_t*)netconfig.get_configblock();
  for (uint8_t i=0; configdesc[i].name!=NULL;++i) {
    void *p = (void*)(pcfgb+configdesc[i].offset);
    if (configdesc[i].isnum) {
      s_param_t val = *(s_param_t*)p;
      Obj.getOrAddMember(configdesc[i].name).set(val);
      //Serial.print(configdesc[i].name);
      //Serial.print(" ");
      //Serial.println(val);
    } else {
      char *text = (char *)p;
      Obj.getOrAddMember(configdesc[i].name).set(text);
      //Serial.print(configdesc[i].name);
      //Serial.print(" ");
      //Serial.println(text);
    }
  }
}

s_param_t c_configitems::update_num_limited(struct s_configdesc * pcb, s_param_t newvalue) {
  uint8_t* pcfgb = (uint8_t*)netconfig.get_configblock();
  void *p = (void*)(pcfgb+pcb->offset);
  if (newvalue > pcb->maxval) {
    newvalue = pcb->maxval;
  }
  if (newvalue < pcb->minval) {
    newvalue = pcb->minval;
  }
  *(s_param_t*)p = newvalue;
  netconfig.mark_update();
  return newvalue;
}


void c_configitems::update_string(struct s_configdesc * pcb, const char * newstr) {
  if (!is_numeric(pcb)) {
    uint8_t* pcfgb = (uint8_t*)netconfig.get_configblock();
    void *p = (void*)(pcfgb+pcb->offset);
    strlcpy((char*) p, newstr, pcb->maxval);
    netconfig.mark_update();
  }
}

void c_configitems::initialize(void){
  uint8_t* pcfgb = (uint8_t*)netconfig.get_configblock();
  for (uint8_t i=0; configdesc[i].name!=NULL;++i) {
    void *p = (void*)(pcfgb+configdesc[i].offset);
    if (configdesc[i].isnum) {
      *(s_param_t*)p = configdesc[i].maxval;
    } else {
      strlcpy((char *)p,"??",5);
    }
  }
}

bool c_configitems::verify_post_load(void){
  uint8_t* pcfgb = (uint8_t*)netconfig.get_configblock();
  for (uint8_t i=0; configdesc[i].name!=NULL;++i) {
    void *p = (void*)(pcfgb+configdesc[i].offset);
    if (configdesc[i].isnum) {
      s_param_t value = *(s_param_t*)p;
      if ((value > configdesc[i].maxval) || (value < configdesc[i].minval)) {
        return false;
      }
    }
  }
  return true;
}
  
