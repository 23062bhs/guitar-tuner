#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <arduinoFFT.h>

#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const int samples = 128; // must be a power of 2 for radix-2 FFT algorithm recursive splitting
const int samplingFrequency = 1024; // sample rate in Hz (reads A0 1024 times per second)

double vReal[samples];
double vImag[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);

const int noiseThreshold = 15;
const float minFreq = 70.0;
const float maxFreq = 400.0;
const double peakDominanceRatio = 2.5;
const double subharmonicEnergyRatio = 0.35;
const unsigned long lockDelayMs = 150;
const int silenceHoldFrames = 8;

bool noteActive = false;
bool locked = false;
double sampleSum = 0;
int sampleCount = 0;
unsigned long noteStartMillis = 0;
int silentFrameCount = 0;

float lastFrequency = -1;
unsigned long samplingPeriodUs;

// LCD display
void drawScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Guitar");
  tft.setCursor(10, 35);
  tft.println("Tuner");
  tft.setTextSize(1);
  tft.setCursor(10, 70);
  tft.println("Play a string");
}

void updateFrequency(float frequency) {
  tft.fillRect(0, 90, 160, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 95);
  tft.print(frequency, 1);
  tft.println(" Hz");
}

void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  drawScreen();

  samplingPeriodUs = round(1000000.0 / samplingFrequency);
}

// parabolic interpolation across neighboring bins for sub-bin accuracy
double refineFrequency(int bin, float binWidth) {
  double y1 = vReal[bin - 1];
  double y2 = vReal[bin];
  double y3 = vReal[bin + 1];
  double d = (y1 - y3) / (2.0 * (y1 - 2.0 * y2 + y3));
  return (bin + d) * binWidth;
}

void finalizeReading() {
  double avgFrequency = sampleSum / sampleCount;
  updateFrequency(avgFrequency);
  lastFrequency = avgFrequency;
  locked = true;
  noteActive = false;
}

void resetForNextNote() {
  noteActive = false;
  locked = false;
  sampleSum = 0;
  sampleCount = 0;
}

void loop() {
  int maxVal = 0;
  int minVal = 1023;

  // pace sampling against micros() so the real sample rate matches samplingFrequency
  unsigned long startMicros = micros();
  for (int i = 0; i < samples; i++) {
    unsigned long targetTime = startMicros + (unsigned long)(i * samplingPeriodUs);
    while (micros() < targetTime) {}
    int sample = analogRead(A0);
    vReal[i] = sample;
    vImag[i] = 0;
    if (sample > maxVal) maxVal = sample;
    if (sample < minVal) minVal = sample;
  }

  int swing = maxVal - minVal;

  if (swing < noiseThreshold) {
    // string may have decayed mid-accumulation - finalize with what we have
    if (noteActive && !locked && sampleCount > 0) {
      finalizeReading();
    }

    // require several consecutive silent frames before clearing the display
    silentFrameCount++;
    if (silentFrameCount >= silenceHoldFrames && lastFrequency != -1) {
      drawScreen();
      lastFrequency = -1;
      resetForNextNote();
    }
    return;
  }

  silentFrameCount = 0;

  // already locked in a reading for this pluck - ignore further frames
  if (locked) {
    return;
  }

  // remove DC bias so it doesn't dominate the spectrum
  double total = 0;
  for (int i = 0; i < samples; i++) total += vReal[i];
  double average = total / samples;
  for (int i = 0; i < samples; i++) vReal[i] -= average;

  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  float binWidth = (float)samplingFrequency / samples;
  int startBin = max(1, (int)(minFreq / binWidth));
  int endBin = min((samples / 2) - 2, (int)(maxFreq / binWidth));

  // find the strongest bin in range, and the average magnitude for the dominance check
  int peakIndex = startBin;
  double peakVal = vReal[startBin];
  double sumVal = 0;
  int count = 0;
  for (int i = startBin; i <= endBin; i++) {
    sumVal += vReal[i];
    count++;
    if (vReal[i] > peakVal) {
      peakVal = vReal[i];
      peakIndex = i;
    }
  }
  double avgVal = sumVal / count;

  // skip this frame if the peak isn't clearly above the noise floor
  if (peakVal < avgVal * peakDominanceRatio) {
    return;
  }

  double frequency = refineFrequency(peakIndex, binWidth);

  // switch to half the frequency if it looks like we locked onto the 2nd harmonic
  double halfFreq = frequency / 2.0;
  if (halfFreq >= minFreq) {
    int halfBin = round(halfFreq / binWidth);
    if (halfBin >= 1 && halfBin <= (samples / 2) - 2) {
      if (vReal[halfBin] > peakVal * subharmonicEnergyRatio) {
        frequency = refineFrequency(halfBin, binWidth);
      }
    }
  }

  // start a new accumulation on the first valid frame of a note
  if (!noteActive) {
    noteActive = true;
    noteStartMillis = millis();
    sampleSum = 0;
    sampleCount = 0;
  }

  sampleSum += frequency;
  sampleCount++;

  // once the window has passed, average and lock in the final reading
  if (millis() - noteStartMillis >= lockDelayMs) {
    finalizeReading();
  }
}
