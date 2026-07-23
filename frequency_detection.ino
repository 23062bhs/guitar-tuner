const int numSamples = 256;
int readings[numSamples];
const int waitTime = 250;

void setup() {
  Serial.begin(9600);
}

void loop() {
  // take a batch of readings from the mic
  for (int i = 0; i < numSamples; i++) {
    readings[i] = analogRead(A0);
    delayMicroseconds(waitTime);
  }

  // find the average and center the wave around 0
  long total = 0;
  for (int i = 0; i < numSamples; i++) {
    total += readings[i];
  }
  int average = total / numSamples;
  for (int i = 0; i < numSamples; i++) {
    readings[i] -= average;
  }

  // find the repeating pattern in the wave
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

  // convert the best shift into a frequency
  if (bestShift > 0) {
    float readingsPerSecond = 1000000.0 / waitTime;
    float frequency = readingsPerSecond / bestShift;
    Serial.print("Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");
  } else {
    Serial.println("No clear pitch detected");
  }
}