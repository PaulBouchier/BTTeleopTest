#include <BTTeleopMediator.h>

BTTeleopMediator::BTTeleopMediator(BTTeleop& btTeleop)
  : btTeleop_(btTeleop)
{
  btTeleop_.setMediator(this);
}
