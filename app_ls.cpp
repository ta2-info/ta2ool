
#include "Headers.h"

extern "C" { void app_lsCommand(Options *options); }

void app_lsCommand(Options *options) {
  const char * bundleID = options->GetNextArgument();
  if (bundleID == nullptr) {
    fprintf(stderr, "Please indicate BundleID.\n");
    return;
  }
  const char * basePath = options->GetNextArgument();
  if (basePath == nullptr) {
    basePath = ".";
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  CFStringRef bundleIDStr = CFStringCreateWithCString(nullptr, bundleID, kCFStringEncodingUTF8);

  AFCConnectionRef afcCnn = nullptr;
  uint32_t resultCode = AMDeviceCreateHouseArrestService(deviceRef, bundleIDStr, nullptr, &afcCnn);
  if (resultCode != 0) {
    printf("Invlid BundleID or Not supported.(0x%08X)\n", resultCode);
  } else {
    listFile(deviceRef, afcCnn, basePath);
    AFCConnectionClose(afcCnn);
    afcCnn = nullptr;
  }

  CFRelease(bundleIDStr);
  bundleIDStr = nullptr;

  ReleaseTargetDevice(deviceRef);
}

