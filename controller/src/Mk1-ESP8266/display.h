#ifndef VENT_DISPLAY_H
  #define VENT_DISPLAY_H

  #include <LiquidCrystal_PCF8574.h>

  #define LCD_I2C_ADDRESS 0x27 // I2C address of PCF8574
  
  LiquidCrystal_PCF8574 display_lcd(LCD_I2C_ADDRESS);  // set the LCD address to 0x27 for a 16 chars and 2 line display


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
 
  } display;







  
  


#endif // VENT_DISPLAY_H
