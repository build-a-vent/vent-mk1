#ifndef VENT_STRINGPARSER_H
#define VENT_STRINGPARSER_H

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


/* word processor - collects incoming characters to words separated by whitespace
 * puts numerical values onto the paramstack, calls word_process for non-numerics */


void word_process(char*,uint8_t);

/* forward declaration of the processor function, on small machines like arduino 
   pointers to functions are a nightmare */


typedef enum { ACK=1,NAK=0,ERR=-1} result_t;

class c_stringparse {
  private:
    #define   TXTBLEN 50
    char buf  [TXTBLEN];
    uint8_t   idx {0};
    bool      negative{false};
    s_param_t accu;

  bool is_decimal(void) {
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
    //Serial.print("push");
    //Serial.println(accu);
    return true;    
  }
      
  public:
    c_stringparse(){idx=0;}
  
      void input(char c) {
        if ((c==' ')||(c=='\n')||(c=='\r')) {
          if (idx>0) {
            buf[idx]=0;
            if (is_decimal()) {
              stack.spush(accu);
            } else {
              word_process(buf,idx);
            }
          }
          idx=0;
        } else {
          if (idx<(TXTBLEN-2)) {
            buf[idx++]=c;
          }
        }
      }
} stringparser; //end class

// stringparser.input(char c) is the method that gets fed the incoming characters


#endif // VENT_STRINGPARSER_H
