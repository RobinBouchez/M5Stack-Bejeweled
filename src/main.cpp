#include <M5StickCPlus.h>
#include <stdlib.h>
#include <stdint.h>
#include "EEPROM.h"

uint32_t black_color = M5.Lcd.color565(0, 0, 0);
uint32_t red_color = M5.Lcd.color565(255, 0, 0);

uint8_t x = 40;
uint8_t y = 10;

uint8_t save_x;
uint8_t save_y;

float pitch;
float yaw;
float roll;

void setup()
{
  M5.begin();
  M5.IMU.Init();
  Serial.begin(115200);
  Serial.flush();
  EEPROM.begin(512);
  M5.Lcd.fillScreen(black_color);
}

void loop()
{
  M5.update();
  if (m5.BtnA.isPressed() && m5.BtnB.isPressed())
  {
    M5.Lcd.fillRect(x, y, 20, 20, black_color);
    x = 40;
    y = 10;
  }

  M5.Lcd.fillRect(x - 20, y, 20, 20, black_color);
  M5.Lcd.fillRect(x, y - 20, 20, 20, black_color);
  M5.Lcd.fillRect(x+20, y, 20, 20, black_color);
  M5.Lcd.fillRect(x, y + 20, 20, 20, black_color);

  M5.Lcd.fillRect(x, y, 20, 20, red_color);

  M5.IMU.getAhrsData(&pitch, &roll, &yaw);
  
   if(roll > 0.8 ) y++;
    else if(roll < -0.8) y--;

   if(x >= m5.Lcd.width()) {
    x =0;
   }
    if(y >= m5.Lcd.height()) {
    y =0;
   }

  if(m5.BtnA.wasPressed()){
    save_x = x;
    save_y = y;
  } else if(m5.BtnB.wasPressed()){
    m5.Lcd.fillScreen(black_color);
    x = save_x;
    y = save_y;
  }

  delay(10);
}
