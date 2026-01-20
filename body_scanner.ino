#include <SoftPWM.h>
#include <SoftPWM_timer.h>

#include <Controllino.h>

const unsigned long stepTime = 1000;     // per LED ON time
const unsigned long allOnTime = 6000;   // all LEDs ON time

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
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], LOW);
  }
}

void loop() {
  unsigned long now = millis();

  switch (state) {

    case FORWARD:
      if (!outputOn) {
        digitalWrite(outputs[index], HIGH);
        previousMillis = now;
        outputOn = true;
      }
      else if (now - previousMillis >= stepTime) {
        digitalWrite(outputs[index], LOW);
        index++;

        if (index >= numOutputs) {
          index = numOutputs - 1;
          state = BACKWARD;
        }

        outputOn = false;
      }
      break;

    case BACKWARD:
      if (!outputOn) {
        digitalWrite(outputs[index], HIGH);
        previousMillis = now;
        outputOn = true;
      }
      else if (now - previousMillis >= stepTime) {
        digitalWrite(outputs[index], LOW);
        index--;

        if (index < 0) {
          state = ALL_ON;
          previousMillis = now;

          for (uint8_t i = 0; i < numOutputs; i++) {
            digitalWrite(outputs[i], HIGH);
          }
        }

        outputOn = false;
      }
      break;

    case ALL_ON:
      if (now - previousMillis >= allOnTime) {
        for (uint8_t i = 0; i < numOutputs; i++) {
          digitalWrite(outputs[i], LOW);
        }

        index = 0;
        state = FORWARD;
      }
      break;
  }
}
