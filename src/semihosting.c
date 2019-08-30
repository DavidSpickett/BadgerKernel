#include "semihosting.h"

unsigned get_semihosting_event(int status) {
  if (status == 0) {
    return 0x20026; //ADP_Stopped_ApplicationExit
  } else {
    return 0x20024; //ADP_Stopped_InternalError
  }
}
