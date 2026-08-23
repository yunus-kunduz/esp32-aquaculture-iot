#pragma once 
#include "ProvDebug.h"
 
void IRAM_ATTR resetbutton_change(void* arg) {  
  // DO NOT CALL ARDUINO SERIAL PRINT INSIDE ISR !!!

  #if BOARD_BUTTON_ACTIVE_LOW
    bool buttonState = !digitalRead(BOARD_BUTTON_PIN);
  #else
    bool buttonState = digitalRead(BOARD_BUTTON_PIN);
  #endif

   ProvState* s = static_cast<ProvState*>(arg);

  if (buttonState && !s->buttonPressed) {
    s->buttonPressTime = millis();
    s->buttonPressed = true;
    s->buttonPressCount +=1;
        
  } else if (!buttonState && s->buttonPressed) {
    s->buttonPressed = false;
    uint32_t buttonHoldTime = millis() - s->buttonPressTime;
    byte buttonPressCount = s->buttonPressCount;
     
    if (buttonHoldTime >= BUTTON_HOLD_TIME_ACTION) {
      ESP.restart();
      while(1){}
    }

    if (buttonPressCount >= BUTTON_INTERRUPT_PRESS_COUNT) {
       s->buttonPressCount = 0; 
       s->state = ProvState::MODE_INTERRUPT;
       return;
    }
     
    s->buttonPressTime = -1;
  }
}

void resetbutton_init() {
  #if BOARD_BUTTON_ACTIVE_LOW
    pinMode(BOARD_BUTTON_PIN, INPUT_PULLUP);
  #else
    pinMode(BOARD_BUTTON_PIN, INPUT);
  #endif
  
  //attachInterrupt(BOARD_BUTTON_PIN, resetbutton_change, CHANGE);
  attachInterruptArg(BOARD_BUTTON_PIN, resetbutton_change, ProvState::getInstance(), CHANGE);
}
