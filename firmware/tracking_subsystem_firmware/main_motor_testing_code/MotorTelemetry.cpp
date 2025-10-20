#include "MotorTelemetry.h"

void MotorTelemetry::begin() {
  analogReadResolution(adcBits);     // UNO R4 supports 12-bit
  // analogReference() not required on UNO R4; adcVref used for math
  pinMode(pinVSense, INPUT);
  pinMode(pinISense, INPUT);
}

void MotorTelemetry::setDivider(float Rtop, float Rbottom) {
  Rtop_ohm = Rtop; Rbottom_ohm = Rbottom;
}

void MotorTelemetry::setCurrentSensor(float sens_V_per_A, float offsetV) {
  sensitivity_V_per_A = sens_V_per_A;
  offset_V = offsetV;
}

inline float MotorTelemetry::_adcToVolts(uint16_t code) const {
  const float fullScale = (float)((1u << adcBits) - 1u);
  return (adcVref * (float)code) / fullScale;
}

inline uint16_t MotorTelemetry::_readOversampled(int pin, uint16_t N) const {
  uint32_t acc = 0;
  for (uint16_t i = 0; i < N; ++i) acc += analogRead(pin);
  return (uint16_t)(acc / N);
}

inline float MotorTelemetry::_vsenseToVm(float vsense) const {
  // Vm = Vadc * (Rtop + Rbottom) / Rbottom
  return vsense * (Rtop_ohm + Rbottom_ohm) / Rbottom_ohm;
}

inline float MotorTelemetry::_isenseToA(float vsense) const {
  // I = (V_meas - offset_V) / sensitivity
  return (vsense - offset_V) / sensitivity_V_per_A;
}

TelemetrySample MotorTelemetry::readOnceRaw() {
  uint16_t codeV = _readOversampled(pinVSense, oversampleN);
  uint16_t codeI = _readOversampled(pinISense, oversampleN);

  float vSenseV = _adcToVolts(codeV);
  float iSenseV = _adcToVolts(codeI);

  float Vm = _vsenseToVm(vSenseV);
  float Ia = _isenseToA(iSenseV);
  TelemetrySample t;
  t.volts = Vm;
  t.amps  = Ia;
  t.watts = Vm * Ia;
  t.us    = micros();
  return t;
}

TelemetrySample MotorTelemetry::readFiltered() {
  TelemetrySample r = readOnceRaw();
  if (!enableIIR) return r;

  if (isnan(_fV)) { _fV = r.volts; _fI = r.amps; _fP = r.watts; }
  else {
    _fV = (1.0f - alpha) * _fV + alpha * r.volts;
    _fI = (1.0f - alpha) * _fI + alpha * r.amps;
    _fP = (1.0f - alpha) * _fP + alpha * r.watts;
  }
  r.volts = _fV; r.amps = _fI; r.watts = _fP;
  return r;
}
