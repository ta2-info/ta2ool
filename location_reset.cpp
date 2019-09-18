
#include <arpa/inet.h>

#include "Headers.h"

extern "C" {
 void location_resetCommand(Options *options);
}


void location_resetCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.dt.simulatelocation"), nullptr, &cnn);
  if (resultCode == 0) {
    uint32_t startMessage = htonl(1);
    AMDServiceConnectionSend(cnn, &startMessage, sizeof(startMessage));
  } else {
    fprintf(stderr, "Cannot start service 'com.apple.dt.simulatelocation'.\n");
  }

  

  ReleaseTargetDevice(deviceRef);
}

