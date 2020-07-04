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
#include "webctrl.h"
#include "config.h"
#include "persist.h"
#include "breathe.h"
#include "stringparser.h"
  
  ESP8266WebServer server (LOCAL_HTTP_PORT);
  c_webcontrol webcontrol; // the object with its webserver port

   
   void c_webcontrol::poll(void) {
      int32_t now=millis();
      r2 = status;
      switch (status) {
        case k_init:
          WiFi.persistent(false);
          WiFi.setAutoConnect(false);
          // read PWD/SSID from EEPROM
          longestop = 0;
          laststep  = now;
          lastbcast = now;
          stepstart = now; 
          if (netconfig.getLoadCks()) {
            char buffer[300];
            sprintf(buffer,"connecting to \"%s\",\"%s\"",netconfig.getSsid(),netconfig.getKey());
            Serial.println(buffer);
            WiFi.begin(netconfig.getSsid(),netconfig.getKey());   // configure the Wi-Fi network you want to connect to
            status=k_start1;
          } else {
            Serial.println("no net configured or damaged, passing to AP mode");
            status=k_start1;
          }
          break;
        case k_start1:
          if ((int32_t)(now-laststep) > 500) { // check all 500 ms for connection to AP
            laststep = now;
            if (WiFi.status() == WL_CONNECTED) {
              status = k_up1;
            }
          } else {
            if ((int32_t)(now-stepstart) > 30000) { // after 10 seconds
              Serial.println("switch to AP Mode");
              WiFi.mode(WIFI_OFF);
              stepstart = now;
              status    = k_start2;
            }
          }
          break;
        case k_start2:
          if ((int32_t)(now-stepstart) > 2000) {
            stepstart = now;
            WiFi.mode(WIFI_AP);
            status    = k_start3;
          }
          break;
        case k_start3:
          if ((int32_t)(now-stepstart) > 2000) {
            stepstart = now;
            if(WiFi.softAP(ApName)){
              Serial.print("\nAP : " + ApName + " running, IP : ");
              Serial.println(WiFi.softAPIP());
              status = k_up2;
            } else {
              Serial.println("AP Mode misfired : "+ApName);
              status = k_start1;
            }
          }
          break;
        case k_up1:        
          if ((int32_t)(now-stepstart) > 2000) {
            MyAddr = WiFi.localIP();
            Netmask = WiFi.subnetMask();
            BroadcastAddr = (uint32_t)MyAddr|((uint32_t)Netmask ^ 0xffffffff);
            Serial.print("Connected to ");
            Serial.println(WiFi.SSID());               // Tell us what network we're connected to
            Serial.print(" IP address: ");
            Serial.print(MyAddr);
            Serial.print(" Netmask: ");
            Serial.print(Netmask);
            Serial.print(" Broadcast: ");
            Serial.println(BroadcastAddr);
            status = k_up3;
            break;
            
          case k_up2:     
            if ((int32_t)(now-stepstart) > 2000) {
              status = k_up3;
            }
            break;
          case k_up3:        
            server.on("/", HTTP_GET, handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
            server.on("/netconf", HTTP_GET, handleNetconf);  // Call the 'handleNetconf' function when a client requests URI "/netconf"
            server.on("/breathconfig", HTTP_GET, handleBreatheView); 
            server.on("/netconfig", HTTP_POST, handleNetconfig);  // Call the 'handleNetconf' function when a client posts to URI "/netconfig"
            server.on("/breathconfig", HTTP_POST, handleBreathePost);
            server.on("/netconfig", HTTP_POST, handleNetconfig);  // Call the 'handleNetconf' function when a client posts to URI "/netconfig"
            server.onNotFound(handleNotFound);           // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
            Serial.println("Starting webserver");
            server.begin();                            // Actually start the server
            Udp.begin(LocalUdpPort);
            status = k_connect;
          }
          break;
        case k_connect : {
            int packetSize = Udp.parsePacket();
            if (packetSize) {
              DynamicJsonDocument reply(2048);
              DynamicJsonDocument request(2048);
              deserializeJson(request,Udp);
              JsonBox.handleIncoming(reply,request);
              if (!reply.isNull()) {
                Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                serializeJson(reply, Udp);
                Udp.println();
                Udp.endPacket();               
              }
              /* ------------------------
              char incomingPacket[512];
              sprintf(incomingPacket,"Received %d bytes from %s, port %d", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
              Serial.println(incomingPacket);
              int len = Udp.read(incomingPacket, 200);
              if (len > 0)
              {
                incomingPacket[len] = '\0';
                Serial.println(incomingPacket);
                if (strncmp(incomingPacket,"ACK:",4) != 0) { // do not reply to ACKs
                  strcat(incomingPacket,":AllnsOkHierBeiUns");
                  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                  Udp.write(incomingPacket, strlen(incomingPacket)+1);
                  Udp.endPacket();
                }
              }
              ------------------ */
              break;
            }
            if ((int32_t)(now - lastbcast) > 5000) {  // broadcast packet every 5 seconds        
              DynamicJsonDocument reply(2048);
              Serial.println("Broadcast");
              Udp.beginPacket(BroadcastAddr, LocalUdpPort);
              JsonBox.fillBroadcastPacket(reply);
              serializeJson(reply, Udp);
              Udp.endPacket();              
              lastbcast=now;
              if (showbcast) {
                --showbcast;
                serializeJson(reply,Serial);
                Serial.println("----");                
              }
              break;
            }
            server.handleClient();                     // Listen for HTTP requests from clients
            // FIXME : check for lost connection ...
          }
          break;
        default:
          Serial.print("unhandled state");
          Serial.println(status);               // Tell us what network we're connected to
          status=k_start2;                    
      }
      int32_t usedup = millis() - now;
      if (usedup > longestop) {
        longestop = usedup;
        r3 = r2;
        r4 = status;
      }
    }

    // commands to manage webservices from Serial
    
    int8_t c_webcontrol::command(char * cmd) {
      if (!strcmp(cmd,"wstat")) {
        static char buffer[100];
        sprintf(buffer," state:%d longest:%d in %d : %d",status,longestop,r3,r4);
        Serial.print("Connected to ");
        Serial.println(WiFi.SSID());               // Tell us what network we're connected to
        Serial.print("IP address:\t");
        Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
        Serial.println(buffer);
        return 1;
      }
      if (!strcmp(cmd,"wclr")) {
        longestop = 0;
        r3 = k_init;
        return 1;
      }
      if (!strcmp(cmd,"wificonfig")) {
        const char *keydata = stringparser.stkget();
        const char *ssiddata = stringparser.stkget();

        if ((keydata != NULL) && (ssiddata != NULL)) {
          netconfig.putSsid(ssiddata);
          netconfig.putKey(keydata);
          netconfig.writeToEeprom();
          Serial.printf("wrote ssid=\"%s\" and key=\"%s\" to eeprom\n",ssiddata,keydata);
          return 1;
        }
      }
      if (!strcmp(cmd,"showbcast")) {
        int32_t n = stack.spop();
        if (n > 0) showbcast=n;
        return 1;
      }

      return 0;
    }

    


// ----------------------------------------- Web pages ----------------------------------------

void handleRoot(void) {                          // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<form action=\"/login\" method=\"POST\">\n"
                                "<input type=\"text\" name=\"username\" placeholder=\"Username\"></br>\n"
                                "<input type=\"password\" name=\"password\" placeholder=\"Password\">\n"
                                "</br><input type=\"submit\" value=\"Login\"></form>\n"
                                "<p>Try 'John Doe' and 'password123' ...</p>");
  server.client().stop();                               
  Serial.println("Rootpage GET");                                
}

void handleNetconfig(void) {                          // When URI /netconfig is POSTed, try to write the WLAN params
  char buffer[500];
  Serial.println("Netconfpost");                                
  if (server.hasArg("ssid") && server.hasArg("key")) {
    netconfig.putSsid(server.arg("ssid").c_str());
    netconfig.putKey(server.arg("key").c_str());
    netconfig.writeToEeprom();
    sprintf(buffer,"<html><h2>successfully set SSID=\"%s\", KEY=\"%s\"</h2><p>now reboot and test</html>",netconfig.getSsid(),netconfig.getKey());
    server.send(200, "text/html", buffer);
  } else {
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
  }
  server.client().stop();                               
}
    
void handleNetconf(void) {                          // When URI /netconf is GET requested, send a web page with input fields
  char buffer[500];   
  Serial.println("Netconf");                                
  if (netconfig.checkCks()) {
    sprintf(buffer,"<form action=\"/netconfig\" method=\"POST\">\n<table>"
                   "<tr><th>SSID</th><td><input type=\"text\" name=\"ssid\" placeholder=\"SSID\" value=\"%s\" minlength=3 maxlength=31></td></tr>\n"
                   "<tr><th>KEY</th><td><input type=\"text\" name=\"key\" placeholder=\"KEY\" value=\"%s\" maxlength=31></td></tr>\n"
                   "<tr><th colspan=2><input type=\"submit\" value=\"netconfig\"></th></tr></table></form>\n",netconfig.getSsid(),netconfig.getKey());
  } else {
    sprintf(buffer,"<form action=\"/netconfig\" method=\"POST\">\n<table>"
                   "<tr><th>SSID</th><td><input type=\"text\" name=\"ssid\" placeholder=\"SSID\" minlength=3 maxlength=31></td></tr>\n"
                   "<tr><th>KEY</th><td><input type=\"text\" name=\"key\" placeholder=\"KEY\" maxlength=31></td></tr>\n"
                   "<tr><th colspan=2><input type=\"submit\" value=\"netconfig\"></th></tr></table></form>\n");
  }
  server.send(200, "text/html", buffer);
  server.client().stop();                               

}



void handleBreatheView(void) {
  char buffer[1000];   
  sprintf(buffer,"<form action=\"/breathconfig\" method=\"POST\">\n<table>"
                   "<tr><th>Flowtime</th><td><input type=\"number\" name=\"flowtime\" placeholder=\"flowtime[ms]\" value=\"%d\" min=300 max=1500></td></tr>\n"
                   "<tr><th>Inhaletime</th><td><input type=\"number\" name=\"inhtime\" placeholder=\"up to plateau[ms]\" value=\"%d\" min=300 max=2500></td></tr>\n"
                   "<tr><th>Cycletime</th><td><input type=\"text\" name=\"cycletime\" placeholder=\"one cycle[ms]\" value=\"%d\" min=2000 max=4000></td></tr>\n"
                   "<tr><th>Running</th><th>%s</th></tr>\n"
                   "<tr><th colspan=2><input type=\"submit\" value=\"start\"></th></tr></table></form>\n",
                   breathe.getFlowtime(),breathe.getPlateautime(),breathe.getCycletime(),breathe.isRunning()?"YES":"NO");
  server.send(200, "text/html", buffer);
  server.client().stop();                               
}

void handleBreathePost(void) {
   char buffer[500];
  Serial.println("BreathPost");                                
  if (server.hasArg("flowtime") && server.hasArg("inhtime") && server.hasArg("cycletime")) {
    s_param_t ft=server.arg("flowtime").toInt();
    s_param_t it=server.arg("inhtime").toInt();
    s_param_t ct=server.arg("cycletime").toInt();
    if ((ft > 300) & (it > ft) && (ct > it)) {
      breathe.start(ft,it,ct);
      handleBreatheView();
    } else {
      server.send(400, "text/plain", "400: Invalid Data");         // The request is invalid, so send HTTP status 400
    }
  } else {
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    server.client().stop();                               
  }
}


void handleNotFound(void){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
  server.client().stop();                               

}
