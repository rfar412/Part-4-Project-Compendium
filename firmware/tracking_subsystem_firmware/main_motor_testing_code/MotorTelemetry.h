#pragma once
#include <Arduino.h>

/*
  MotorTelemetry – Arduino UNO R4
  Reads motor SUPPLY voltage & current and computes power.
  - Voltage via divider on analog pin
  - Current via Hall sensor (ACS712) on analog pin
  - Uses 12-bit ADC (UNO R4), moving-average filter, and oversampling

  ***** WIRING SETUP *****
  Vm (motor +) --> Rtop --> (A_VSENSE) --> Rbottom --> GND
  Motor supply return (or sensor output) --> (A_ISENSE)


*/

struct TelemetrySample {
  float volts;   // motor supply voltage [V]
  float amps;    // motor supply current [A]
  float watts;   // volts * amps [W]
  uint32_t us;   // timestamp micros()
};

class MotorTelemetry {
public:
  int   pinVSense = A0;     // analog pin for voltage divider
  int   pinISense = A1;     // analog pin for current sensor

  // ADC
  uint8_t  adcBits = 12;           // UNO R4 = 12-bit
  float    adcVref = 5.0f;         // default analog ref (UNO R4 = 5V)
  uint16_t oversampleN = 16;       // per reading (power of two preferred)

  // Voltage divider (Vm = Vadc * (Rtop+Rbottom)/Rbottom)
  float Rtop_ohm    = 47000.0f;    // e.g., 47k
  float Rbottom_ohm = 10000.0f;    // e.g., 10k

  // Current sensor (Model Setup)
  // B) ACS712-5A    sensitivity_V_per_A = 0.185 V/A, offset_V ~ Vref/2 (≈2.5V on 5V ref)
  float sensitivity_V_per_A = 0.185 V/A, offset_V ~ Vref/2 (≈2.5V on 5V ref)
  float offset_V            = 2.5f;   // [V] output at 0 A (ACS712 midpoint). Set 0 for unipolar shunt.

  // Filtering
  float alpha = 0.2f;  // IIR low-pass 
  bool  enableIIR = true;

  // ---- API ----
  void begin();
  TelemetrySample readOnceRaw();     // single oversampled, unfiltered
  TelemetrySample readFiltered();    // IIR filtered
  void setDivider(float Rtop, float Rbottom);
  void setCurrentSensor(float sens_V_per_A, float offsetV);

private:
  float _fV = NAN, _fI = NAN, _fP = NAN;
  inline float _adcToVolts(uint16_t code) const;
  inline uint16_t _readOversampled(int pin, uint16_t N) const;
  inline float _vsenseToVm(float vsense) const;
  inline float _isenseToA(float vsense) const;
};
