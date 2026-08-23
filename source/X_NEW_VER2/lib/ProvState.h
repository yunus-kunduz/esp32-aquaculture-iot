#pragma once 

class ProvState {
    private:
      static ProvState* instance;
      ProvState();

    public:
      static ProvState* getInstance();
      
      enum State {
        MODE_WAIT_WIFI_CONFIG,
        MODE_CONNECTING_WIFI,
        MODE_WIFI_CONNECTED_WAIT_APP_CREDENTIALS,
        MODE_SWITCH_TO_STA,
        MODE_PROV_DONE,
        MODE_RESET_CONFIG,
        MODE_ERROR,
        MODE_INTERRUPT
      };           

      volatile State state; 
      volatile bool     buttonPressed = false;
      volatile uint32_t buttonPressTime = -1;
      volatile byte     buttonPressCount = 0; // how many times the button has been pressed 

};

/* Null, because instance will be initialized on demand. */
ProvState* ProvState::instance = 0;

ProvState* ProvState::getInstance() {
  if (instance == 0){
      instance = new ProvState();
  }

  return instance;
}

ProvState::ProvState() {}
