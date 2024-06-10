
#include <arpa/inet.h>

#include "Headers.h"

extern "C" {
 void syslogCommand(Options *options);
}


void syslogCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    fprintf(stderr, "Device not found...\n");
    return;
  }

  CFMutableDictionaryRef myDict = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(myDict, CFSTR("UnlockEscrowBag"), kCFBooleanTrue);

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.syslog_relay"), myDict, &cnn);
  if (resultCode == 0) {
    AMDeviceStopSession(deviceRef);

    setbuf(stdout, nullptr);
    char buffer[4096];
    while (true) {
      memset(buffer, 0, sizeof(buffer));
      int readLen = AMDServiceConnectionReceive(cnn, buffer, sizeof(buffer) - 1);
      if (readLen > 0) {
        if (buffer[0] == 0) {
          printf("%s", buffer + 1);
        } else {
          printf("%s", buffer + 0);
        }
      }
    }
  } else {
    fprintf(stderr, "Cannot start service 'com.apple.syslog_relay'.\n");
  }

  

  ReleaseTargetDevice(deviceRef);
}

