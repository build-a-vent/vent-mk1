
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

#include "stringparser.h"
#include "pstk.h"

  bool c_stringparse::is_decimal(void) {
    uint8_t p=0;
    accu=0; negative=false;
    if (buf[0]=='+') {
      p=1;
    }
    if (buf[0]=='-') {
      p=1; negative=true;
    }
    for (;p<idx;++p) {
      char c=buf[p];
      if ((c>='0') && (c<='9')) {
        accu = accu*10 + (c-'0');
      } else {
        return false;
      }
    }
    if (negative) { 
      accu = -accu;
    }
    return true;    
  }
      
  
void c_stringparse::input(char c) {
  if ((c==' ')||(c=='\n')||(c=='\r')) {
    if (idx>0) {
      buf[idx]=0;
      if (is_decimal()) {
        stack.spush(accu);
      } else {
        if (buf[0]=='"') {
          if (stkp < STKLEN) {
            strcpy(wstack[stkp++].b,buf+1);
          } else {
            Serial.println("wstack ovfl");
          }
        } else {
          word_process(buf,idx);
        }
      }
    }
    idx=0;
  } else {
    if (idx<(TXTBLEN-2)) {
      buf[idx++]=c;
    }
  }
}

const char * c_stringparse::stkget(void) {
  if (stkp) {
    return wstack[--stkp].b;
  }
  return NULL;
}

void c_stringparse::stkdrop(void) {
  if (stkp) stkp--;
}

void c_stringparse::stkclr(void) {
  stkp=0;
}

int8_t c_stringparse::command(char *cmd) {
  if (!strcmp(cmd,"wstkclr")) { 
    stkclr();
    return 1;
  }
  if (!strcmp(cmd,"wstkp")) { 
    int i;
    for (i=stkp-1;i>=0;--i) {
      Serial.printf("%d \"%s\"\n",i,wstack[i].b);
    }
    Serial.printf("--tot %d-\n",stkp);
    return 1;
  }
  return 0;
}

      
c_stringparse stringparser;
