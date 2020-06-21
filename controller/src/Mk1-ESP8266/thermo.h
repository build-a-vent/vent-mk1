#ifndef VENT_THERMO_H
#define VENT_THERMO_H

// a DS18B20 based thermosensor object, requires OneWire

class thermosensor_s_B20 {   // single 18B20 on a OneWire Bus.
  
  public:
  
    thermosensor_s_B20(OneWire& iWire) : myWire(iWire) {}

    thermosensor_s_B20() = delete;
    
    bool startconversion(void) {
      if (!myWire.reset()) {
        return false;
      }
      myWire.skip(); // skip rom, we assume  only a single device on the bus
      myWire.write(0x44); // start conversion command, takes up to 750ms
      return true;
    }
    
    bool endconversion(int16_t* pCelsius) {
      uint8_t data[9];
      if (!myWire.reset()) {
        return false;
      }
      myWire.skip();
      myWire.write(0xBE);         // Read Scratchpad
    
      for (uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = myWire.read();
      }
    
      if ((uint8_t)OneWire::crc8(data, 8) != data[8]) {
        return false;
      }
    
      // Convert the data bytes to actual temperature, omit fractional bits
      *pCelsius = (data[1] << 4) | data[0]>>4;
      return true;
    }
    
  private:
  
    OneWire & myWire;
};

class thermocontrol_2p {
  private:
    typedef enum { k_init         = 0,
                   k_start        = 1, 
                   k_await_t1     = 2, 
                   k_await_t2     = 3, 
                   k_fail         = 4,
                   k_restart      = 5
                 } kState;
    void setheater(bool v) {
      heateron = v;
      digitalWrite(pwrPin,v ? HIGH : LOW);
    }
                       
  public:
    thermocontrol_2p(thermosensor_s_B20 & bottle, thermosensor_s_B20 & air, uint8_t pin) : 
      sensBottle(bottle), sensAir(air), pwrPin(pin) {
        pinMode(pwrPin,OUTPUT);
        setheater(false);
        state=k_init;
    }
  
    thermocontrol_2p() = delete;

    void settemp(int16_t t) {
      c_tempBottleSet=t;
    }

    void setalert(int16_t t) {
      c_tempBottleAlert=t;
    }
    
  
    int8_t command(const char *cmd) {
      if (!strcmp(cmd,"tempset")) { 
        s_param_t temp = stack.spop();

        if ((temp > 0) && (temp < 40)) {
          settemp(temp);
          return 1;
        }
        return -1;
      }

      if (!strcmp(cmd,"alertset")) { 
        s_param_t temp = stack.spop();

        if ((temp > 0) && (temp < 40)) {
          setalert(temp);
          return 1;
        }
        return -1;
      }

      if (!strcmp(cmd,"tempshow")) { 
        Serial.print ("Status ");
        Serial.print (state);
        Serial.print (", TempBottle ");
        Serial.print (a_tempBottle);
        Serial.print (", TempAir ");
        Serial.println (a_tempAir);
        return 1; 
      }
      
      return 0;
      
    }

    void poll(void) {
      int32_t now = millis();
      int16_t newt;
      switch (state) {
        case k_start :
          // FIXME : Schalten nur in unkritischen phasen
          state = k_await_t1;
          nextstep = now + 1000;
          if (!sensAir.startconversion()) {
            state=k_fail;
          }
          if (!sensBottle.startconversion()) {
            state=k_fail;
          }
          break;
        case k_await_t1:
          if ((now - nextstep) > 0) {
            state = k_await_t2;
            if (!sensAir.endconversion(&a_tempAir)) {
              state = k_fail;
            }
          }
          break;
        case k_await_t2:
          if (!sensBottle.endconversion(&a_tempBottle)) {
            state = k_fail;
          } else {
            setheater(c_tempBottleSet > a_tempBottle);
            state = k_restart;
          }
          break;
        case k_fail :
          setheater(false);
          nextstep=now+2000;
          state=k_restart;
          break;
        case k_restart :
          if ((now - nextstep) > 0) {
            state=k_start;
          }
          break;
        default:
        case k_init :
          setheater(false);
          state=k_start;
      }
    }


  private:
    kState state;
    thermosensor_s_B20 & sensBottle;
    thermosensor_s_B20 & sensAir;
    uint8_t              pwrPin;
    int16_t              a_tempBottle;      // actual bottle temp
    int16_t              a_tempAir;         // actual air temp
    int16_t              c_tempBottleAlert; // Bottle alert temperature
    int16_t              c_tempBottleSet;   // Bottle Temp Setpoint    
    int32_t              nextstep; // when does the next operation take part        
    bool                 heateron;
  
}; // end class







#endif // VENT_THERMO_H
