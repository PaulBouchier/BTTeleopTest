#pragma once

#include <Arduino.h>
#include <ArduinoLog.h>

#include <BluetoothSerial.h>

#include <cstdlib>
#include <string>

using namespace std;

// String bt_id = "Bot-BT";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

class Mediator;

class BTTeleop
{
public:
  BTTeleop();
  bool init(int logLevel, Stream* logStream=NULL); // initialize BTTeleop, passing a stream pointer for logging
  void run(void* params);       // BTTeleop task starts running here
  void getBTState(float& vel, float& rotSpeed, bool& teleopActive, bool& enableAutoRun);
  void setMediator(Mediator* mediator) { mediator_ = mediator; }

  Logging btTeleopLog_;
  UBaseType_t getStackHighWaterMark() { return uxTaskGetStackHighWaterMark(btTeleopTaskHandle_);}

private:
  void execBtCmd();  // process incoming BT command

  BluetoothSerial serialBt_;
  const string bt_id_ = string("Mowbot-BT");
  char cmd_[100];
  int cmdIndex_;
  float maxSpeed_ = 0.35;    // m/s
  float maxRotSpeed_ = 0.3;  // rad/s
  float axf_, ayf_, azf_;      // acceleration in x/y/z

  // Motor controller should query BTTeleop and process motion commands (if any) if enableAutoRun_ is true and
  // ignore motion control commands if enableAutoRun_ is false. Motor controller should process teleop commands
  // and ignore computer motion commands while teleopActive is true.
  int lastTeleopTime_;          // last time we received a teleop cmd. 
  int lastEnableAutoRunTime_;   // last time we received an enableAutoRun cmd.
  bool buttonActive_ = false;   // when true, Enable Auto Run button (with its watchdog) must be true to enable running autonomously 
  int nextPongTime_;            // when robot should send "alive" to RoboRemo

  TaskHandle_t btTeleopTaskHandle_ = NULL;
  Mediator* mediator_ = NULL;
};