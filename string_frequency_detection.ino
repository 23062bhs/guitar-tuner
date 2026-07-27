#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const int numSamples = 256;
int readings[numSamples];
const int waitTime = 250;
const int noiseThreshold = 50;

void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Guitar Tuner");
  tft.setCursor(10, 40);
  tft.println("Play a string");
}

void loop() {
  // take a batch of readings from the mic
  for (int i = 0; i < numSamples; i++) {
    readings[i] = analogRead(A0);
    delayMicroseconds(waitTime);
  }

  // find the average and center the wave around 0
  long total = 0;
  for (int i = 0; i < numSamples; i++) total += readings[i];
  int average = total / numSamples;
  for (int i = 0; i < numSamples; i++) readings[i] -= average;

  // check if signal is loud enough to be a real note
  int maxVal = 0;
  for (int i = 0; i < numSamples; i++) {
    if (abs(readings[i]) > maxVal) maxVal = abs(readings[i]);
  }
  if (maxVal < noiseThreshold) {
    // if the note is too quiet, show waiting message
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Guitar Tuner");
    tft.setCursor(10, 40);
    tft.println("Play a string...");
    return;
  }

  // autocorrelation
  long bestMatchScore = 0;
  int bestShift = 0;
  for (int shift = 4; shift < numSamples / 2; shift++) {
    long matchScore = 0;
    for (int i = 0; i < numSamples - shift; i++) {
      matchScore += (long)readings[i] * readings[i + shift];
    }
    if (matchScore > bestMatchScore) {
      bestMatchScore = matchScore;
      bestShift = shift;
    }
  }

  // convert shift to frequency and display it
  if (bestShift > 0) {
    float readingsPerSecond = 1000000.0 / waitTime;
    float frequency = readingsPerSecond / bestShift;

    // display frequency on screen
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Guitar Tuner");
    tft.setCursor(10, 50);
    tft.print("Freq: ");
    tft.print(frequency, 1);  // 1 decimal place
    tft.println(" Hz");
  }
}
