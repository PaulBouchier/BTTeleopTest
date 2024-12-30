#include <BTTeleop.h>
#include <LogStream.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

extern BTTeleop btTeleop;

// @brief static function to call run() method
void static startBtTeleopTask(void* params) { btTeleop.run(params); }

// Prefix print function for logs from this object
static void printPrefix(Print* logOutput, int logLevel)
{
  switch(logLevel)
  {
    default:
    case 0: logOutput->print("S: BtTeleop: "); break;
    case 1: logOutput->print("F: BtTeleop: "); break;
    case 2: logOutput->print("E: BtTeleop: "); break;
    case 3: logOutput->print("W: BtTeleop: "); break;
    case 4: logOutput->print("I: BtTeleop: "); break;
    case 5: logOutput->print("T: BtTeleop: "); break;
    case 6: logOutput->print("V: BtTeleop: "); break;
  }
}

BTTeleop::BTTeleop()
{
}

bool
BTTeleop::init(int logLevel, Stream* stream_p)
{
  bool isok = true;

  // Start logger
  if (stream_p == NULL)
  {  
    // No logStream provided, use logStream
    LogStream* logStream_p = new LogStream();
    logStream_p->setMediator(mediator_);
    btTeleopLog_.begin(logLevel, logStream_p);
  }
  else
  {
    btTeleopLog_.begin(logLevel, stream_p);
  }
  btTeleopLog_.setPrefix(printPrefix);
  btTeleopLog_.setShowLevel(false);

  btTeleopLog_.infoln("BTTeleop::init()");

  serialBt_.begin(bt_id_.c_str());  // start Bluetooth Serial
  btTeleopLog_.infoln("The device with name %s started, now you can pair it with bluetooth!", bt_id_.c_str());
  nextPongTime_ = millis() + 500;

  // create BTTeleop task
  BaseType_t rv = xTaskCreate(
                    startBtTeleopTask,
                    "BtTeleop Task",
                    4096,
                    NULL,
                    1,
                    &btTeleopTaskHandle_);

  // hang here forever if task creation failed
  while(rv != pdTRUE)
  {
    Serial.println("Failed to create BtTeleop task; stopped");
    btTeleopLog_.fatalln("Failed to create BtTeleop task; stopped");
    delay(2000);   // periodically print the failed message
  }

  return isok;
}

void
BTTeleop::getBTState(float& vel, float& rotSpeed, bool& teleopActive, bool& enableAutoRun)
{
  // default returns if no buttonActive_ or teleopActive commands received
  teleopActive = false;
  enableAutoRun = true;
  vel = 0.0;
  rotSpeed = 0.0;

  if ((millis() - lastTeleopTime_) < 1000)
  {
    teleopActive = true;   // teleopActive lasts 1 sec after last last "to" msg is received
    enableAutoRun = false;   // ignore autonomous cmds during teleop

    // teleop linear speed based on last received accelerometer readings
    if (ayf_ < 0)
      vel = maxSpeed_;
    else if (azf_ < 0)
      vel = -maxSpeed_;
    else if (abs(azf_ - ayf_) > 1.0)
    {
      float velFwd = azf_ / (ayf_ + azf_);
      float velBackwd = ayf_ / (ayf_ + azf_);
      if (velFwd > 0.55)
        vel = ((velFwd * 2) - 1.0) * maxSpeed_;
      else if (velBackwd > 0.55)
        vel = -(((velBackwd * 2) - 1.0) * maxSpeed_);
    }

    // teleop rotational speed based on last received accelerometer readings
    if (axf_ > 5.0)
      axf_ = 5.0;
    else if (axf_ < -5.0)
      axf_ = -5.0;
    if (abs(axf_) > 0.5)
      rotSpeed = maxRotSpeed_ * axf_ / 5.0;
  }
  else if (buttonActive_ && (millis() - lastEnableAutoRunTime_ > 1000))
  {
    // autonomous running is disabled if buttonActive_ is true and button is not pressed
    enableAutoRun = false;
  }
}

void
BTTeleop::execBtCmd()
{
  // process incoming BT command
  string rcvString = string(cmd_);
  // btTeleopLog_.infoln(rcvString.c_str());
  string cmdString = rcvString.substr(0, 2);
  float velFwd;
  float velBackwd;

  if (string("ax") == cmdString) {
    axf_ = atof(&cmd_[3]);
  } else if (string("ay") == cmdString) {
    ayf_ = atof(&cmd_[3]);
  } else if (string("az") == cmdString) {
    azf_ = atof(&cmd_[3]);
  } else if (string("to") == cmdString) {
    lastTeleopTime_ = millis();
  } else if (string("hb") == cmdString) {
    buttonActive_ = false;  // teleop screen heartbeat, set buttonActive_ to 0, not relevant in this mode
  } else if (string("ea") == cmdString) {
    lastEnableAutoRunTime_ = millis();
  } else if (string("ba") == cmdString) {
    if (atoi(&cmd_[3]) == 0)  // get buttonActive_ state
      buttonActive_ = false;
    else
      buttonActive_ = true;
  }
}

void
BTTeleop::run(void* params)
{
  btTeleopLog_.infoln("Running BTTeleopTask");

  // loop forever in the task, keeping BT traffic flowing
  while (true)
  {
    // get BT command & process it
    while (serialBt_.available())
    {
      char c = serialBt_.read();
      if (c == '\n')
      {
        cmd_[cmdIndex_] = '\0';
        execBtCmd();
        cmdIndex_ = 0;
      }
      else
      {
        cmd_[cmdIndex_] = c;
        if (cmdIndex_ < 99)
          cmdIndex_++;
      }
    }

    // send "alive" to RoboRemo every 500ms
    if (millis() > nextPongTime_)
    {
      char hiString[] = "hi 1\n\0";
      for (int i = 0; i < 5; i++)
      {
        serialBt_.write(hiString[i]);
      }
      nextPongTime_ = millis() + 500;
    }

    vTaskDelay(50);
  }
}