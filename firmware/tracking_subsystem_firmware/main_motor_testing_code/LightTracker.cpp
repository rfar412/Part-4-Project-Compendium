#include "LightTracker.h"

void LightTracker::begin() {
  analogReadResolution(adcBits);
  pinMode(pinPD, INPUT);
  pinMode(pinLDRL, INPUT);
  pinMode(pinLDRR, INPUT);
  _lastCheckMs = millis();
}

uint16_t LightTracker::_readOversampled(int pin, uint16_t N) const {
  uint32_t acc = 0;
  for (uint16_t i = 0; i < N; ++i) acc += analogRead(pin);
  return (uint16_t)(acc / N);
}

inline float LightTracker::_adcToVolts(uint16_t code) const {
  const float full = (float)((1u << adcBits) - 1u);
  return adcVref * (float)code / full;
}

void LightTracker::_updateFilters() {
  auto samp = [&](int pin, bool invert)->float {
    float v = _adcToVolts(_readOversampled(pin, oversampleN));
    if (invert) v = adcVref - v;
    return constrain(v, 0.0f, adcVref);
  };

  float vpd = samp(pinPD,   invertPD) + offsetPD_V;
  float vl  = samp(pinLDRL, invertL)  + offsetLDRL_V;
  float vr  = samp(pinLDRR, invertR)  + offsetLDRR_V;

  if (isnan(_fPD)) { _fPD = vpd; _fL = vl; _fR = vr; }
  else {
    _fPD = (1-alpha)*_fPD + alpha*vpd;
    _fL  = (1-alpha)*_fL  + alpha*vl;
    _fR  = (1-alpha)*_fR  + alpha*vr;
  }
}

TrackDecision LightTracker::decide() {
  const uint32_t now = millis();
  if (_inSettle) {
    if (now >= _settleUntilMs) _inSettle = false;
    else return TrackDecision::Hold;
  }
  if ((now - _lastCheckMs) < checkPeriodMs) return TrackDecision::Hold;
  _lastCheckMs = now;

  _updateFilters();

  // With our wiring, brighter -> higher volts on all channels
  // Normalize using PD; algebra reduces to V_R - V_L, but we keep the form:
  float eL = _fL - _fPD;
  float eR = _fR - _fPD;
  float err = eR - eL;   // +ve => RIGHT brighter
  _lastErr = err;

  float thUp   = deadband_V + hysteresis_V;
  float thDown = deadband_V;

  TrackDecision d = TrackDecision::Hold;
  if (err > thUp)          d = TrackDecision::Right;
  else if (err < -thUp)    d = TrackDecision::Left;
  else if (err > thDown)   d = (_lastDecision == TrackDecision::Right) ? TrackDecision::Right : TrackDecision::Hold;
  else if (err < -thDown)  d = (_lastDecision == TrackDecision::Left)  ? TrackDecision::Left  : TrackDecision::Hold;

  if (d == _lastDecision && d != TrackDecision::Hold) _sameDecisionCount++;
  else if (d != TrackDecision::Hold) _sameDecisionCount = 1;
  else _sameDecisionCount = 0;

  if (d != TrackDecision::Hold && _sameDecisionCount >= stableCountReq) {
    _lastDecision = d;
    _sameDecisionCount = 0;
    _inSettle = true;
    _settleUntilMs = now + settleAfterMoveMs;
    return d;
  }
  if (d != TrackDecision::Hold) _lastDecision = d;
  return TrackDecision::Hold;
}

// NEW: quick on-pose calibration to remove tiny L/R biases
void LightTracker::calibrate(uint16_t ms) {
  uint32_t t0 = millis();
  float sumPD=0, sumL=0, sumR=0; uint16_t n=0;
  while (millis() - t0 < ms) {
    float vpd = _adcToVolts(_readOversampled(pinPD,   oversampleN));
    float vl  = _adcToVolts(_readOversampled(pinLDRL, oversampleN));
    float vr  = _adcToVolts(_readOversampled(pinLDRR, oversampleN));
    if (invertPD) vpd = adcVref - vpd;
    if (invertL)  vl  = adcVref - vl;
    if (invertR)  vr  = adcVref - vr;
    sumPD += vpd; sumL += vl; sumR += vr; n++;
    delay(2);
  }
  float mPD = sumPD/n, mL = sumL/n, mR = sumR/n;

  // Make the current pose neutral: (mR - mPD) == (mL - mPD)  -> offsets trim to equalize
  // We keep PD offset 0 and split the correction across L and R:
  float diff = (mR - mL) * 0.5f;
  offsetLDRL_V +=  diff;
  offsetLDRR_V += -diff;

  // Reset filters to the trimmed means so we don’t jump after cal
  _fPD = mPD + offsetPD_V;
  _fL  = mL  + offsetLDRL_V;
  _fR  = mR  + offsetLDRR_V;
}
