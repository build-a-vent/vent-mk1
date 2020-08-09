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
#include "config.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "json.h"

#if HAS_WEBSERVER
  #include <ESP8266WebServer.h>
  // fwd dcl of webserver handler routines
  void handleRoot(void);
  void handleNetconf(void);
  void handleNetconfig(void);
  void handleBreatheView(void);
  void handleBreathePost(void);
  void handleNotFound(void);
#endif


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
    int32_t          showbcast{0};
    int32_t          netshowrx{0};
    int32_t          netshowtx{0};
    WiFiUDP          Udp;
    IPAddress        MyAddr;
    IPAddress        Netmask;
    IPAddress        BroadcastAddr;
    byte             mac[6];                     // the MAC address of your Wifi shield
    String           ApName; 
    String           MacId;


  public:

    c_webcontrol() {
      WiFi.macAddress(mac);
      MacId  = WiFi.macAddress();
      ApName = "bav_"+MacId;
    }

    inline void setup(void) {
      Serial.println(String("WifiModule has mac ") + MacId); 
      #if LOGALOTMORE
        Serial.print("MAC: ");
        Serial.print(mac[0],HEX);
        Serial.print(":");
        Serial.print(mac[1],HEX);
        Serial.print(":");
        Serial.print(mac[2],HEX);
        Serial.print(":");
        Serial.print(mac[3],HEX);
        Serial.print(":");
        Serial.print(mac[4],HEX);
        Serial.print(":");
        Serial.println(mac[5],HEX);
      #endif
    }


    inline void add_json(JsonObject &Obj) {
        Obj.getOrAddMember("mac").set(MacId);
    }

    inline bool verify_mac(const char *json_mac) {
      return !strcmp(json_mac,MacId.c_str());
    }

    void poll(void);
    int8_t command(char * cmd);
        

};

extern c_webcontrol webcontrol;

#endif // VENT_WEBCONTROL_H
