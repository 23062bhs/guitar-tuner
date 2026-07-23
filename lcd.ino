#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {

  Serial.begin(9600);

  tft.initR(INITR_GREENTAB);

  tft.setRotation(1);

  // black screen
  tft.fillScreen(ST77XX_BLACK);

  // white border
  tft.drawRect(0, 0, 160, 128, ST77XX_WHITE);

  // Text 
  tft.setTextColor(ST77XX_YELLOW); 
  tft.setTextSize(2); tft.setCursor(15, 20); 
  tft.println("Hello!"); 
  tft.setTextColor(ST77XX_CYAN); 
  tft.setCursor(15, 50); 
  tft.println("Arduino"); 
  delay(2000);

  // colour test
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(10, 40);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.println("Display");
  tft.setCursor(10, 65);
  tft.println("Working");

}

void loop() {

}






