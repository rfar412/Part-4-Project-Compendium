/*
  NEMA 17 Azimuth (0–270° in 15° steps) – Arduino UNO R4
  Driver: A4988 (STEP/DIR/EN), microstepping = 1/16

  - moveToAzimuth(angleDeg): moves to nearest 15° tick within [0, 270]
  - Maintains currentAzimuthDeg
  - Uses rounding for deg→steps to avoid cumulative error
*/

const int PIN_STEP   = 2;
const int PIN_DIR    = 3;
const int PIN_ENABLE = 4;

// A4988 microstep pins (1/16 microstep = HIGH,HIGH,HIGH)
const int PIN_MS1 = 5;
const int PIN_MS2 = 6;
const int PIN_MS3 = 7;

const unsigned long STEP_HIGH_US = 800;   
const unsigned long STEP_LOW_US  = 800;
const bool ENABLE_LOW_ACTIVE     = true;  // A4988: LOW = enable

// --- Geometry / conversion ---
const int  FULL_STEPS_PER_REV = 200;   // 1.8° per full step
const int  MICROSTEPPING      = 16;  
const long STEPS_PER_REV      = (long)FULL_STEPS_PER_REV * MICROSTEPPING;


const bool USE_STEPS_PER_270 = false;
const long STEPS_PER_270     = 15;

const int AZIMUTH_STEP_DEG = 15;       // Valid Commands 15,30,45,60,75,90,105,120,135,150,165,180,195,210,225,240,255,270. 

int currentAzimuthDeg = 0;             // assume home at 0° (Initial Location of motor device)

// --------- Helpers ----------
void driverEnable(bool enable) {
  digitalWrite(PIN_ENABLE, enable ? (ENABLE_LOW_ACTIVE ? LOW : HIGH)
                                  : (ENABLE_LOW_ACTIVE ? HIGH : LOW));
}

long round_divide(long num, long den) {
  // Symmetric rounding for signed integers (round to nearest)
  if (num >= 0) return (num + den/2) / den;
  else          return (num - den/2) / den;
}

// Convert a delta angle (can be negative) into steps
long azimuthDegreesToSteps(int deltaDeg) {
  if (USE_STEPS_PER_270) {
    // steps = deltaDeg * (STEPS_PER_270 / 270)  with rounding
    return round_divide((long)deltaDeg * STEPS_PER_270, 270L);
  } else {
    // steps = deltaDeg * (STEPS_PER_REV / 360) with rounding
    return round_divide((long)deltaDeg * STEPS_PER_REV, 360L);
  }
}

void stepN(long steps) {
  if (steps == 0) return;
  digitalWrite(PIN_DIR, (steps >= 0) ? HIGH : LOW);
  long N = labs(steps);
  for (long i = 0; i < N; i++) {
    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(STEP_HIGH_US);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(STEP_LOW_US);
  }
}

int clampAndQuantizeAngle(float requestedDeg) {
  if (requestedDeg < 0)   requestedDeg = 0;
  if (requestedDeg > 270) requestedDeg = 270;
  int ticks = (int)round(requestedDeg / AZIMUTH_STEP_DEG);
  return ticks * AZIMUTH_STEP_DEG;
}

// --------- API ----------
void moveToAzimuth(float targetDeg) {
  int target   = clampAndQuantizeAngle(targetDeg);
  int deltaDeg = target - currentAzimuthDeg;
  long steps   = azimuthDegreesToSteps(deltaDeg);

  driverEnable(true);
  stepN(steps);
  driverEnable(false);

  currentAzimuthDeg = target;
}

// --------- Optional serial control ----------
void handleSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n'); cmd.trim();

  if (cmd.startsWith("AZ ")) {
    float angle = cmd.substring(3).toFloat();
    moveToAzimuth(angle);
    Serial.print("Moved to azimuth: ");
    Serial.print(currentAzimuthDeg);
    Serial.println(" deg");
  } else if (cmd == "POS?") {
    Serial.print("Azimuth at: ");
    Serial.print(currentAzimuthDeg);
    Serial.println(" deg");
  } else {
    Serial.println("Commands: 'AZ <angleDeg>' (0..270), 'POS?'");
  }
}

// --------- Setup / Loop ----------
void setup() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);

  // Set 1/16 microstepping on A4988: HIGH,HIGH,HIGH
  pinMode(PIN_MS1, OUTPUT);
  pinMode(PIN_MS2, OUTPUT);
  pinMode(PIN_MS3, OUTPUT);
  digitalWrite(PIN_MS1, HIGH);
  digitalWrite(PIN_MS2, HIGH);
  digitalWrite(PIN_MS3, HIGH);

  driverEnable(false);

  Serial.begin(115200);
  Serial.println("Azimuth stepper ready (1/16 microstep). Use: 'AZ <angleDeg>' or 'POS?'");
}

void loop() {
  handleSerial();

  // // quick demo (comment out in production):
  // moveToAzimuth(0);   delay(500);
  // moveToAzimuth(90);  delay(500);
  // moveToAzimuth(180); delay(500);
  // moveToAzimuth(270); delay(500);
}
// Voltage and Current Reading Block
\
#include "MotorTelemetry.h"
MotorTelemetry tel;

// ===== in setup() =====
void setup() {
  // ... your existing motor setup ...
  Serial.begin(115200);

  tel.begin();

  // --- SET THESE TO MATCH YOUR HARDWARE ---
  // Example divider: 47k over 10k → scale ≈ 5.7x (reads up to ~28.5V at 5V ADC)
  tel.setDivider(47000.0f, 10000.0f);

  // Example: ACS712-5A (0.185 V/A, 2.5V at 0A). For shunt+amp use offsetV=0 and your V/A.
  tel.setCurrentSensor(0.185f, 2.5f);

  tel.oversampleN = 16;   // 8–32 is a good range
  tel.alpha       = 0.2f; // smoothing 0.1–0.3 typical
  tel.enableIIR   = true;

  Serial.println("Telemetry ready.");
}

#include "LightTracker.h"
LightTracker tracker;


// config: allowed tick size
const int AZIMUTH_MIN_DEG  = 0;
const int AZIMUTH_MAX_DEG  = 270;

uint32_t lastTrackPrintMs = 0;

void setup() {
  // ... your existing setup (pins, MS pins, Serial, tel.begin(), etc.)
  tracker.begin();

  // Optionally tweak thresholds if your sensors are very close together:
  // tracker.deadband_V = 0.03f;
  // tracker.hysteresis_V = 0.015f;
  // tracker.stableCountReq = 2;
}

void loop() {
  handleSerial();  

  // ---- Tracking logic ----
  TrackDecision d = tracker.decide();
  if (d == TrackDecision::Left) {
    int tgt = max(AZIMUTH_MIN_DEG, currentAzimuthDeg - AZIMUTH_STEP_DEG);
    if (tgt != currentAzimuthDeg) moveToAzimuth(tgt);
  } else if (d == TrackDecision::Right) {
    int tgt = min(AZIMUTH_MAX_DEG, currentAzimuthDeg + AZIMUTH_STEP_DEG);
    if (tgt != currentAzimuthDeg) moveToAzimuth(tgt);
  }

  // (Optional) print debug
  if (millis() - lastTrackPrintMs >= 250) {
    lastTrackPrintMs = millis();
    Serial.print("PD=");
    Serial.print(tracker.Vpd(), 3);
    Serial.print("V  L=");
    Serial.print(tracker.Vl(), 3);
    Serial.print("V  R=");
    Serial.print(tracker.Vr(), 3);
    Serial.print("V  err=");
    Serial.print(tracker.Verr(), 3);
    Serial.print("V  dir=");
    Serial.println((d==TrackDecision::Left)?"LEFT":(d==TrackDecision::Right)?"RIGHT":"HOLD");
  }


}

// after tracker.begin();
delay(300);
tracker.calibrate(800);  // ~0.8s average with the rig pointed at the sun

// In your serial handler (add):
else if (cmd == "CAL") {
  tracker.calibrate(800);
  Serial.print("Cal done. Offsets L=");
  Serial.print(tracker.Vl() - tracker.Vpd(), 3);
  Serial.print("  R=");
  Serial.println(tracker.Vr() - tracker.Vpd(), 3);
}
