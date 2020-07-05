#ifndef VENT_DISPLAY_H
  #define VENT_DISPLAY_H
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

  #if SIMULATE_VENT

  #else
    #include <Wire.h>
    #include <LiquidCrystal_PCF8574xxx.h>
  
    #define LCD_I2C_ADDRESS 0x27 // I2C address of PCF8574
    
    extern LiquidCrystal_PCF8574 display_lcd;
  
    class lcd_display {
      private:
   
      public:
  
        void initialize(void) {
          Wire.beginTransmission(LCD_I2C_ADDRESS);
          int error = Wire.endTransmission();
          display_lcd.begin(20, 4); // initialize the lcd 20 cols, 4 rows
          display_lcd.setBacklight(255);
          display_lcd.clear();
          display_lcd.home();
          display_lcd.display();
          
        }
        void actualize(uint8_t row, uint8_t col, char*buffer) {
          display_lcd.setCursor(col,row);
          display_lcd.print(buffer);
        }
   
    };
  
    extern class lcd_display display;
  #endif
#endif // VENT_DISPLAY_H
