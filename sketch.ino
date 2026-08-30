#include <Servo.h>

const int POT_PIN = A0;       // Simulates raw muscle contraction level
const int SERVO_PIN = 9;      // Prosthetic finger actuator
const int LED_PIN = 13;       // Threshold activation LED

Servo prostheticFinger;

// DSP Moving RMS Buffer Parameters
const int WINDOW_SIZE = 25;
float squareBuffer[WINDOW_SIZE];
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  prostheticFinger.attach(SERVO_PIN);
  
  for (int i = 0; i < WINDOW_SIZE; i++) {
    squareBuffer[i] = 0.0;
  }
}

void loop() {
  // 1. Read base muscle effort (0 - 1023)
  int baseEffort = analogRead(POT_PIN);

  // 2. Synthesize noisy raw EMG bio-potential signal
  float timeSec = millis() / 1000.0;
  float noise60Hz = 70.0 * sin(2.0 * M_PI * 60.0 * timeSec); // Powerline interference
  float muscleNoise = (random(-50, 50) * (baseEffort / 1023.0)); // Random muscle fiber bursts
  
  // Center raw signal around 512 (2.5V DC offset)
  float rawEMG = 512.0 + noise60Hz + muscleNoise;

  // 3. DSP Step A: Remove DC Offset
  float centeredSignal = rawEMG - 512.0;

  // 4. DSP Step B: Rectification & Square for RMS Window
  squareBuffer[bufferIndex] = centeredSignal * centeredSignal;
  bufferIndex = (bufferIndex + 1) % WINDOW_SIZE;

  // 5. DSP Step C: Compute Moving Root Mean Square (RMS) Envelope
  float sumSquares = 0.0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sumSquares += squareBuffer[i];
  }
  float rmsEnvelope = sqrt(sumSquares / WINDOW_SIZE);

  // 6. Actuator Control: Drive Servo proportional to extracted RMS envelope
  int servoAngle = map(constrain(rmsEnvelope, 0, 150), 0, 150, 0, 180);
  prostheticFinger.write(servoAngle);

  // Trigger alert LED if contraction surpasses threshold
  digitalWrite(LED_PIN, rmsEnvelope > 35.0 ? HIGH : LOW);

  // 7. Output formatted values for the Wokwi Serial Plotter
  Serial.print("Raw_Noisy_EMG:");
  Serial.print(rawEMG);
  Serial.print(",");
  Serial.print("Extracted_RMS_Envelope:");
  Serial.println(rmsEnvelope * 2.0 + 512.0); // Offset for visual overlay

  delay(10); // 100 Hz sampling rate
}