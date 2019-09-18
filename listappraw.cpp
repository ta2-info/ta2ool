
#include "Headers.h"

extern "C" { void listapprawCommand(Options *options); }

void listapprawCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  CFDictionaryRef result = nullptr;
  uint32_t resultCode = AMDeviceLookupApplications(deviceRef, nullptr, &result);
  if (resultCode == 0 && result != nullptr) {
    CFShow(result);
  }

  if (result != nullptr) {
    CFRelease(result);
    result = nullptr;
  }

  ReleaseTargetDevice(deviceRef);
}

