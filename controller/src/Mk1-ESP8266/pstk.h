#ifndef VENT_PARAMSTACK_H
#define VENT_PARAMSTACK_H
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
#include "stdio.h"
/*
 * an implementation of a parameter stack
 * needs predefined s_param_t
 * implements the globally visible stack object
 */

class c_paramstk {
    #define MAXSTK 50
    s_param_t stacka[MAXSTK];
    uint8_t sp{0};
  public:  
  
    c_paramstk() {
      sp=0;
    }
    
    uint8_t getsp() { return sp; }
    
    s_param_t speek(void) { return (sp) ? stacka[sp-1] : -1; }
    
    s_param_t speek(uint8_t delta) { return (sp>delta) ? stacka[sp-delta-1] : -1; }
    
    void sinsert(s_param_t z, uint8_t delta) { 
      if ((sp > delta) && (sp < (MAXSTK-2))) {
        for (uint8_t i=0;i<delta;++i) {
          uint8_t from=sp-i-1;
          stacka[from+1]=stacka[from];
        } stacka[sp-delta]=z;
      }
    }
    
    void srot(uint8_t delta) { 
      if ((sp > delta)) {
        s_param_t a=stacka[sp-1];
        for (uint8_t i = 0; i < delta; ++i) {
          uint8_t from=sp-i-2;
          stacka[from+1]=stacka[from];
        } stacka[sp-delta-1]=a;
      }
    }
    
    s_param_t spop(void) { 
      return (sp) ? stacka[--sp] : -1; 
    }
    
    void spush(s_param_t a) {
      if (sp<MAXSTK) {
        stacka[sp++]=a;
      }
    }
    
    void sclear() { sp=0; }  
    
    void slevel() 
    {  stacka[sp]=sp; 
       sp++;
    }

    void prnstk(void);
    

    
    int8_t command(char *cmd);

    
};

extern c_paramstk stack;

#endif // defined VENT_PARAMSTACK_H
