#ifndef VENT_JSON_H
#define VENT_JSON_H
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
#include <ArduinoJson.h>

  class c_JsonBox {
    public:
      c_JsonBox() {};
  
      void handleIncoming(JsonDocument &Reply,JsonDocument &Request);
      void fillBroadcastPacket(JsonDocument &Doc);
      int8_t command(char *cmd);


  };

  extern c_JsonBox JsonBox;

#endif
