#include <BTTeleopMediator.h>
#include <BTTeleop.h>

#include <Arduino.h>
#include "BluetoothSerial.h"

// Globals
BTTeleop btTeleop;
//BTTeleopMediator btTeleopMediator(btTeleop);

void setup() {
  bool isok = true;
  Serial.begin(115200);
  delay(3000);  // Wait a moment to start (so we don't miss Serial output)

  isok = btTeleop.init(LOG_LEVEL_INFO, &Serial);
  delay(1000);      // wait for startup messages from BtTeleop object to get printed
  if (isok)
    btTeleop.btTeleopLog_.infoln("Successfully initialized btTeleop");
}

void loop() {
  float vel;
  float rotSpeed;
  bool teleopActive;
  bool enableAutoRun;

  btTeleop.getBTState(vel, rotSpeed, teleopActive, enableAutoRun);
  Serial.printf("vel: %f, rotSpeed: %f, teleopActive: %d, enableAutoRun: %d\n", vel, rotSpeed, teleopActive, enableAutoRun);

  vTaskDelay(500);
}

