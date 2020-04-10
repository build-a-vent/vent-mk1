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

/*
 * an implementation of a parameter stack
 * needs predefined s_param_t
 * implements the globally visible stack object
 */

class paramstack {
    #define MAXSTK 50
    s_param_t stacka[MAXSTK];
    uint8_t sp{0};
  public:  
  
    paramstack() {
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

    void prnstk() { 
      char buffer[20];
      sprintf(buffer,"STK[%d]",sp); Serial.print(buffer);
      for (int8_t i=sp-1;i>=0;--i) {
        sprintf(buffer," %d",stacka[i]); Serial.print(buffer);
      }
      Serial.println();
    }

    int8_t command(char *cmd) {
      // spush is done implicitely by presenting a number

      if (!strcmp(cmd,"sclr")) { sp=0; return 1; }

      if (!strcmp(cmd,".s")) { prnstk(); return 1; }

      if (!strcmp(cmd,"level")) { slevel(); return 1; }

      if (!strcmp(cmd,"drop")) { 
        if (sp > 0) { 
          --sp;
          return 1;
        } else {
          return -1; //ERR
        }
      }

      if (!strcmp(cmd,"rot")) { 
        if (sp > 2) { 
          srot(2); return 1;
        }
        return -1; //ERR
      }
      return 0;

      if (!strcmp(cmd,"nrot")) { 
        if (sp > 2) { 
          srot(spop()); return 1;
        }
        return -1; //ERR
      }
      return 0;
    }

  
    
} stack;

#endif // defined VENT_PARAMSTACK_H
