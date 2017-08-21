#include "init.h"
#include "pwm.h"

int main(int argc, char** argv)
{
  ALT_STATUS_CODE status = system_init();

  doPwm();

  return 0;
}
