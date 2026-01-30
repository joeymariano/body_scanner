#include <avr/wdt.h>
#include <SoftPWM.h>
#include <SoftPWM_timer.h>
#include <Controllino.h>

/* ---------------- Timing ---------------- */

unsigned long fadeTime    = 1000;
unsigned long holdTime    = 2000;
unsigned long allOnTime   = 4000;
unsigned long allOffTime  = 2000;

unsigned long previousMillis = 0;
int loopCount = 0;

/* ---------------- State Machine ---------------- */

enum State {
  FORWARD,
  BACKWARD,
  ALL_ON,
  ALL_OFF
};

enum Phase {
  FADE_IN,
  HOLD,
  FADE_OUT
};

State state = FORWARD;
Phase phase = FADE_IN;
int index = 0;

/* ---------------- Outputs ---------------- */

const uint8_t outputs[] = {
  CONTROLLINO_D0,
  CONTROLLINO_D1,
  CONTROLLINO_D2,
  CONTROLLINO_D3,
  CONTROLLINO_D4,
  CONTROLLINO_D5
};

const uint8_t numOutputs = sizeof(outputs) / sizeof(outputs[0]);

/* ---------------- Helpers ---------------- */

void applyFadeTimes() {
  for (uint8_t i = 0; i < numOutputs; i++) {
    SoftPWMSetFadeTime(outputs[i], fadeTime, fadeTime);
  }
}

bool systemHealthy() {
  return true;
}

/* ---------------- Setup ---------------- */

void setup() {
  Serial.begin(9600);
  SoftPWMBegin();

  wdt_disable();
  wdt_enable(WDTO_2S);

  for (uint8_t i = 0; i < numOutputs; i++) {
    SoftPWMSet(outputs[i], 0);
  }

  applyFadeTimes();
}

/* ---------------- Loop ---------------- */

void loop() {
  unsigned long now = millis();

  /* ---- Attract vs Normal Mode ---- */
  if (loopCount % 3 == 0) {
    fadeTime   = 50;
    holdTime   = 25;
    allOnTime  = 250;
    allOffTime = 100;
  } else {
    fadeTime   = 1200;
    holdTime   = 2000;
    allOnTime  = 4000;
    allOffTime = 2000;
  }

  applyFadeTimes();

  /* ---- State Machine ---- */

  switch (state) {

    case FORWARD:
    case BACKWARD:

      switch (phase) {

        case FADE_IN:
          SoftPWMSet(outputs[index], 255);
          previousMillis = now;
          phase = HOLD;
          break;

        case HOLD:
          if (now - previousMillis >= fadeTime + holdTime) {
            SoftPWMSet(outputs[index], 0);
            previousMillis = now;
            phase = FADE_OUT;
          }
          break;

        case FADE_OUT:
          if (now - previousMillis >= fadeTime) {

            if (state == FORWARD) {
              index++;
              if (index >= numOutputs) {
                index = numOutputs - 2;
                state = BACKWARD;
              }
            } else {
              index--;
              if (index < 0) {
                state = ALL_ON;
                previousMillis = now;
                for (uint8_t i = 0; i < numOutputs; i++) {
                  SoftPWMSet(outputs[i], 255);
                }
                break;
              }
            }

            phase = FADE_IN;
          }
          break;
      }
      break;

    case ALL_ON:
      if (now - previousMillis >= allOnTime) {
        for (uint8_t i = 0; i < numOutputs; i++) {
          SoftPWMSet(outputs[i], 0);
        }
        previousMillis = now;
        state = ALL_OFF;
      }
      break;

    case ALL_OFF:
      if (now - previousMillis >= allOffTime) {
        index = 0;
        phase = FADE_IN;
        state = FORWARD;
        loopCount++;
        if(loopCount >= 1000) {
          loopCount = 0;
          }
        Serial.println(loopCount);
      }
      break;
  }

  /* ---- Watchdog ---- */
  if (systemHealthy()) {
    wdt_reset();
  }
}
