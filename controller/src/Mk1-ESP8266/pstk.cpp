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

#include "pstk.h"
#include "Arduino.h"

void c_paramstk::prnstk() { 
  char buffer[20];
  sprintf(buffer,"STK[%d]",sp); Serial.print(buffer);
  for (int8_t i=sp-1;i>=0;--i) {
    sprintf(buffer," %d",stacka[i]); Serial.print(buffer);
  }
  Serial.println();
}


int8_t c_paramstk::command(char *cmd) {
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

c_paramstk stack;
