#ifndef VENT_WEBCONTROL_H
#define VENT_WEBCONTROL_H
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
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include "json.h"


void handleRoot(void);
void handleNetconf(void);
void handleNetconfig(void);
void handleBreatheView(void);
void handleBreathePost(void);
void handleNotFound(void);


class c_webcontrol {

  typedef enum { k_init    = 0,
                 k_start1  = 1,
                 k_start2  = 2,
                 k_start3  = 3,
                 k_up1     = 4,
                 k_up2     = 5,
                 k_up3     = 6,
                 k_connect = 7
               } t_stat;

  private :
    t_stat           status{k_init};
    t_stat           r2, r3, r4;
    int32_t          laststep;
    int32_t          stepstart;
    int32_t          lastbcast;
    int32_t          longestop{0};
    WiFiUDP          Udp;
    IPAddress        MyAddr;
    IPAddress        Netmask;
    IPAddress        BroadcastAddr;
    byte             mac[6];                     // the MAC address of your Wifi shield
    String           ApName; 
    String           MacId;

    //ESP8266WebServer &server;    // ref to the web server object, passed in ctor


  public:

    c_webcontrol() {
      ApName = "bav_"+WiFi.macAddress();
      MacId  = ApName.substring(13);
    }

    inline void setup(void) {
      Serial.println(String("WifiModule has mac ") + MacId);
    }


    inline void add_json(JsonObject &Obj) {
        Obj.getOrAddMember("f_macid").set(MacId);
    }

    void poll(void);
    int8_t command(char * cmd);
    
    // forward declarations for web page handlers
    

};

extern c_webcontrol webcontrol;

#endif // VENT_WEBCONTROL_H
