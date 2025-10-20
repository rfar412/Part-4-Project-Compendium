#pragma once
#include <Arduino.h>

// Direction decision for motor
enum class TrackDecision : uint8_t { Hold = 0, Left = 1, Right = 2 };

class LightTracker {
public:
  // ---- Pins (set wiring) ----
  int pinPD   = A2; // Photodiode (reference intensity)
  int pinLDRL = A3; // LDR left
  int pinLDRR = A4; // LDR right

  // ---- ADC / sampling ----
  uint8_t  adcBits       = 12;   // UNO R4
  float    adcVref       = 5.0f; // math reference (typ 5V)
  uint16_t oversampleN   = 16;   // 8–32 good
  float    alpha         = 0.25; // IIR smoothing (0..1)

  // ---- Decision logic ----
  float deadband_V       = 0.05f;  // no move if |err| < 50 mV
  float hysteresis_V     = 0.02f;  // 20 mV stickiness once moving
  uint8_t stableCountReq = 3;      // require N consecutive same decisions
  uint16_t checkPeriodMs = 60;     // how often we re-evaluate
  uint16_t settleAfterMoveMs = 250;// wait after each move for sensors to settle

  // ---- Calibration (optional additive offsets in volts) ----
  float offsetPD_V   = 0.0f;
  float offsetLDRL_V = 0.0f;
  float offsetLDRR_V = 0.0f;

  // ---- Public API ----
  void begin();
  // Call periodically; returns Hold/Left/Right based on PD-normalized L–R error
  TrackDecision decide();

  // Last filtered readings (for debugging/telemetry)
  float Vpd() const   { return _fPD; }
  float Vl()  const   { return _fL;  }
  float Vr()  const   { return _fR;  }
  float Verr() const  { return _lastErr; }

private:
  // filters
  float _fPD = NAN, _fL = NAN, _fR = NAN;
  float _lastErr = 0.0f;
  uint32_t _lastCheckMs = 0;
  TrackDecision _lastDecision = TrackDecision::Hold;
  uint8_t _sameDecisionCount = 0;
  bool _inSettle = false;
  uint32_t _settleUntilMs = 0;

  // internals
  uint16_t _readOversampled(int pin, uint16_t N) const;
  inline float _adcToVolts(uint16_t code) const;
  void _updateFilters();
};
