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
#include "configitems.h"
#include <WiFiUdp.h>
#include "json.h"
#include "webctrl.h"
#include "breathe.h"

int8_t c_JsonBox::command(const char * const cmd) {
  if (!strcmp(cmd,"jshow")) {
    DynamicJsonDocument Eins(1024);
    JsonObject Rep = Eins.to<JsonObject>();
    c_configitems::serialize_all(Rep);
    serializeJson(Rep,Serial);
    Serial.println();
    return 1;
  }
  return 0;
}

void c_JsonBox::fillBroadcastPacket(JsonDocument &Doc) {
  JsonObject Obj = Doc.to<JsonObject>();
  Obj.getOrAddMember("cmd").set("status");  
  c_configitems::serialize_scan(Obj);
  webcontrol.add_json(Obj);
  breathe.add_current_status(Obj);
}

int c_JsonBox::saveConfigurables(JsonObject &ReplyObj, JsonObject &SrcObj) {
  int count=0;
  for (JsonPair kv : SrcObj) {
    const char * argname = kv.key().c_str();
    struct s_configdesc *pcd = c_configitems::get_cfgdesc_by_name(argname);
    if (pcd) { // this param exists
      ++count;
      if (c_configitems::is_numeric(pcd)) {
        s_param_t val = kv.value().as<int>();
        Serial.println("Update "+String(argname)+" to " + String(val));
        s_param_t rc = c_configitems::update_num_limited(pcd,val);
        ReplyObj.getOrAddMember(argname).set(rc);
      } else {
        const char * nv = kv.value().as<char*>();
        c_configitems::update_string(pcd,nv);
      }
    }
  }
  return count;
}


void c_JsonBox::handleIncoming(JsonDocument &Reply, JsonDocument &Request) {
  JsonObject ObjReq = Request.as<JsonObject>();
  JsonObject RepO = Reply.to<JsonObject>();
  const char* cmd = ObjReq["cmd"];
  if (!strcmp("scan",cmd)) {
    ObjReq.getOrAddMember("req").set(cmd);
    fillBroadcastPacket(Reply);
    return;
  }
  // all following commands require the correct mac sent by the Android app
  const char* jmac = ObjReq["mac"];
  if (jmac) {
    if (webcontrol.verify_mac(jmac)) {
      if (!strcmp("set",cmd)) {
        //serializeJson(Request,Serial);
        webcontrol.add_json(RepO);
        RepO.getOrAddMember("cmd").set("ack");
        RepO.getOrAddMember("req").set(cmd);
        #if 0
        for (JsonPair kv : ObjReq) {
          const char * argname = kv.key().c_str();
          struct s_configdesc *pcd = c_configitems::get_cfgdesc_by_name(argname);
          if (pcd) { // this param exists
            if (c_configitems::is_numeric(pcd)) {
              s_param_t val = kv.value().as<int>();
              Serial.println("Update "+String(argname)+" to " + String(val));
              s_param_t rc = c_configitems::update_num_limited(pcd,val);
              RepO.getOrAddMember(argname).set(rc);
            } else {
              const char * nv = kv.value().as<char*>();
              c_configitems::update_string(pcd,nv);
            }
          }
        }
        #endif
        saveConfigurables(RepO,ObjReq);
      }
      if (!strcmp("configmode",cmd)) {
        //Breather.configmode();
        Serial.println ("Seeing configmode");
        RepO.getOrAddMember("req").set(cmd);
        RepO.getOrAddMember("mac").set(jmac);
        RepO.getOrAddMember("cmd").set("configuring");
      }
      if (!strcmp("valvecfg",cmd)) {
        //Breather.valvecfg(ObjReq["action"]);
        Serial.println ("Seeing valvecfg");
        RepO.getOrAddMember("req").set(cmd);
        RepO.getOrAddMember("mac").set(jmac);
        RepO.getOrAddMember("cmd").set("valverun");
      }
      if (!strcmp("config",cmd)) {
        Serial.println ("Seeing config");
        RepO.getOrAddMember("req").set(cmd);
        RepO.getOrAddMember("mac").set(jmac);
        RepO.getOrAddMember("cmd").set("configack");
        saveConfigurables(RepO,ObjReq);

      }
    }
  }
}

c_JsonBox JsonBox;
