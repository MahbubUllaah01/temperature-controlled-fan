/*
 * Temperature-Controlled Fan Speed System
 * Board  : Arduino Uno
 * Sensor : LM35 (10 mV per degree Celsius) on A0
 * Output : PWM pin 9 -> MOSFET/transistor gate -> DC fan
 *
 * The fan stays off in cool conditions, then speeds up smoothly
 * as the temperature rises, and runs at full speed when hot.
 */

const int SENSOR_PIN = A0;
const int FAN_PIN    = 9;       // PWM-capable pin

const float TEMP_OFF  = 28.0;   // fan turns OFF below this (deg C)
const float TEMP_ON   = 30.0;   // fan turns ON above this (deg C)
const float TEMP_FULL = 40.0;   // full speed at or above this (deg C)

const int PWM_MIN = 90;         // lowest duty that reliably spins the fan
const int PWM_MAX = 255;

const int SAMPLES = 10;         // readings averaged to reduce noise
const unsigned long LOOP_DELAY_MS = 500;

bool fanRunning = false;

float readTemperatureC() {
  long total = 0;
  for (int i = 0; i < SAMPLES; i++) {
    total += analogRead(SENSOR_PIN);
    delay(5);
  }
  float adc = total / (float)SAMPLES;
  float volts = adc * (5.0 / 1023.0);
  return volts * 100.0;         // LM35: 10 mV per deg C
}

int speedFromTemperature(float t) {
  if (t >= TEMP_FULL) return PWM_MAX;
  return (int)map((long)(t * 10), (long)(TEMP_ON * 10), (long)(TEMP_FULL * 10), PWM_MIN, PWM_MAX);
}

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, 0);
  Serial.begin(9600);
  Serial.println(F("Temperature-controlled fan started"));
}

void loop() {
  float t = readTemperatureC();

  // Hysteresis: separate ON and OFF thresholds stop the fan from flickering
  if (!fanRunning && t >= TEMP_ON)  fanRunning = true;
  if (fanRunning  && t <= TEMP_OFF) fanRunning = false;

  int pwm = fanRunning ? constrain(speedFromTemperature(t), PWM_MIN, PWM_MAX) : 0;
  analogWrite(FAN_PIN, pwm);

  Serial.print(F("Temp: "));
  Serial.print(t, 1);
  Serial.print(F(" C | Fan PWM: "));
  Serial.print(pwm);
  Serial.print(F(" ("));
  Serial.print(pwm * 100 / 255);
  Serial.println(F("%)"));

  delay(LOOP_DELAY_MS);
}
