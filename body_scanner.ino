#include <SoftPWM.h>
#include <SoftPWM_timer.h>
#include <Controllino.h>

const unsigned long stepTime = 4000;     // per LED ON time
const unsigned long allOnTime = 8000;    // all LEDs ON time
const unsigned long fadeTime = 1000;      // fade in/out time

unsigned long previousMillis = 0;

enum State {
  FORWARD,
  BACKWARD,
  ALL_ON
};

State state = FORWARD;

int index = 0;
bool outputOn = false;

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
    SoftPWMSet(outputs[i], 0);  // start OFF
    SoftPWMSetFadeTime(outputs[i], fadeTime, fadeTime);
  }
}

void loop() {
  unsigned long now = millis();

  switch (state) {

    case FORWARD:
      if (!outputOn) {
        SoftPWMSet(outputs[index], 255);  // fade ON
        previousMillis = now;
        outputOn = true;
      }
      else if (now - previousMillis >= stepTime) {
        SoftPWMSet(outputs[index], 0);    // fade OFF
        index++;

        if (index >= numOutputs) {
          index = numOutputs - 2;
          state = BACKWARD;
        }

        outputOn = false;
      }
      break;

    case BACKWARD:
      if (!outputOn) {
        SoftPWMSet(outputs[index], 255);  // fade ON
        previousMillis = now;
        outputOn = true;
      }
      else if (now - previousMillis >= stepTime) {
        SoftPWMSet(outputs[index], 0);    // fade OFF
        index--;

        if (index < 0) {
          state = ALL_ON;
          previousMillis = now;

          for (uint8_t i = 0; i < numOutputs; i++) {
            SoftPWMSet(outputs[i], 255);  // fade ALL ON
          }
        }

        outputOn = false;
      }
      break;

    case ALL_ON:
      if (now - previousMillis >= allOnTime) {
        for (uint8_t i = 0; i < numOutputs; i++) {
          SoftPWMSet(outputs[i], 0);      // fade ALL OFF
        }

        index = 0;
        state = FORWARD;
      }
      break;
  }
}
