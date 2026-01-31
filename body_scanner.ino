#include <avr/wdt.h>
#include <SoftPWM.h>
#include <SoftPWM_timer.h>
#include <Controllino.h>

/* ---------------- Logarithmic Lookup Table ---------------- */
// Maps linear input (0-255) to logarithmic output for natural brightness
const uint8_t PROGMEM gammaTable[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
    2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,
    5,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,
   10,  10,  11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,
   17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
   25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  35,  36,
   37,  38,  39,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  50,
   51,  52,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  66,  67,  68,
   69,  70,  72,  73,  74,  75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,
   90,  92,  93,  95,  96,  98,  99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

/* ---------------- Helper to apply gamma correction ---------------- */
uint8_t applyGamma(uint8_t value) {
  return pgm_read_byte(&gammaTable[value]);
}

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
  CONTROLLINO_D4
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
          SoftPWMSet(outputs[index], applyGamma(255));  // Apply gamma correction
          previousMillis = now;
          phase = HOLD;
          break;
          
        case HOLD:
          if (now - previousMillis >= fadeTime + holdTime) {
            SoftPWMSet(outputs[index], applyGamma(0));  // Apply gamma correction
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
                  SoftPWMSet(outputs[i], applyGamma(255));  // Apply gamma correction
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
          SoftPWMSet(outputs[i], applyGamma(0));  // Apply gamma correction
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