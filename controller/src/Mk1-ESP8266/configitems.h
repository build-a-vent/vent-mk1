#ifndef VENT_CONFIGITEMS_H
#define VENT_CONFIGITEMS_H
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

#include <cstddef>
#include <ArduinoJson.h>


  #define CFGNAMELEN 16

   // configuration data in FLASH storage  

  struct s_configblock {
    s_param_t c_pip;     // config   peak inspir press alarm mbar
    s_param_t c_lip;     // config lowest inspir press alarm mbar
    s_param_t c_pep;     // config   peak exspir press alarm mbar
    s_param_t c_lep;     // config lowest exspir press alarm mbar
    s_param_t c_flair;   // config flow rate air     ml/sec
    s_param_t c_flo2;    // config flow rate o2      ml/sec
    
    s_param_t c_airt;    // config per-cycle air open time ms
    s_param_t c_o2t;     // config per-cycle o2 open time  ms
    s_param_t c_inspt;   // config per cycle inspir. time  ms
    s_param_t c_cyclt;   // config cycle time ms
    
    s_param_t c_wtemp;   // config water temperature degC

    char      c_name[CFGNAMELEN]; // config : vent name
  };

struct s_configdesc {
    char* const name;
    uint16_t  offset;
    s_param_t minval;
    s_param_t maxval; 
    bool      isnum;   
};

#define CFGOFFS(EXP) (offsetof(struct s_configblock,EXP  ))
  
class c_configitems {
  private: 
  public:
    static struct      s_configdesc * get_cfgdesc_by_name(const char * const pname);
    inline static bool is_numeric(struct s_configdesc * pcb) { return pcb->isnum; }
    static s_param_t   update_num_limited(struct s_configdesc * pcb, s_param_t newvalue);
    static void        update_string(struct s_configdesc * pcb, const char * newstr);
    static uint8_t     serialize_config(JsonObject &Obj);
    static void        initialize(void);
    static bool        verify_post_load(void);
    static int8_t      command(const char *cmd);


};


#endif // VENT_CONFIGITEMS_H
 
