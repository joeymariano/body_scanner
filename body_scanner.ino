#include <SoftPWM.h>
#include <SoftPWM_timer.h>
#include <Controllino.h>

const unsigned long fadeTime   = 1000;   // fade in/out time
const unsigned long holdTime   = 4000;  // fully ON time
const unsigned long allOnTime  = 6000;

unsigned long previousMillis = 0;

enum State {
  FORWARD,
  BACKWARD,
  ALL_ON
};

enum Phase {
  FADE_IN,
  HOLD,
  FADE_OUT
};

State state = FORWARD;
Phase phase = FADE_IN;

int index = 0;

const uint8_t outputs[] = {
  CONTROLLINO_D0,
  CONTROLLINO_D1,
  CONTROLLINO_D2,
  CONTROLLINO_D3,
  CONTROLLINO_D4,
  CONTROLLINO_D5
};

const uint8_t numOutputs = sizeof(outputs) / sizeof(outputs[0]);

void setup() {
  SoftPWMBegin();

  for (uint8_t i = 0; i < numOutputs; i++) {
    SoftPWMSet(outputs[i], 0);
    SoftPWMSetFadeTime(outputs[i], fadeTime, fadeTime);
  }
}

void loop() {
  unsigned long now = millis();

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
          if (now - previousMillis >= holdTime) {
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
                return;
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
        index = 0;
        phase = FADE_IN;
        state = FORWARD;
      }
      break;
  }
}
