#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {

  Serial.begin(9600);

  // initialise the display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  // colour test
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  delay(500);

  // clear the screen
  tft.fillScreen(ST77XX_BLACK);
  // display  message
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(15, 40);
  tft.println("Display");
  tft.setCursor(15, 65);
  tft.println("Working");
}

void loop() {

}
