#pragma once

#include <Mediator.h>
#include <BTTeleop.h>

class BTTeleopMediator : public Mediator
{
public:
  BTTeleopMediator(BTTeleop& btTeleop);

  // dummy defs to staisfy Mediator
  void publishOdometry(OdometryMsg odom);
  void clearOdometry() {}
  void setWheelDirections(bool leftFwd, bool rightFwd) {}
  void sendLogMsg(char* logMsg, int length) {}
  void setDrive(int32_t seq, float linear_vel, float angular_vel) {}
  void setLogLvl(int32_t pilinkLogLevel, int32_t rl500LogLevel, int32_t odomLogLevel) {}
  void publishPlatformData(PlatformDataMsg platformData) {}
  void setRL500BITMode() {}
  void getBTState(float& vel, float& rotSpeed, bool& teleopActive, bool& enableAutoRun);

private:
  BTTeleop& btTeleop_;

};