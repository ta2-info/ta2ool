
#include "Headers.h"

extern "C" { void app_pullCommand(Options *options); }

void app_pullCommand(Options *options) {
  const char * bundleID = options->GetNextArgument();
  if (bundleID == nullptr) {
    fprintf(stderr, "Please indicate BundleID.\n");
    return;
  }
  const char * targetPath = options->GetNextArgument();
  if (targetPath == nullptr) {
    fprintf(stderr, "パスが指定されていない\n");
    return;
  }

  // 省略した場合は nullptr
  const char * localPathOrDir = options->GetNextArgument();

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  CFStringRef bundleIDStr = CFStringCreateWithCString(nullptr, bundleID, kCFStringEncodingUTF8);

  AFCConnectionRef afcCnn = nullptr;
  uint32_t resultCode = AMDeviceCreateHouseArrestService(deviceRef, bundleIDStr, nullptr, &afcCnn);
  if (resultCode != 0) {
    printf("Invlid BundleID or Not supported.\n");
  } else {
    pullFile(deviceRef, afcCnn, targetPath, localPathOrDir);
    AFCConnectionClose(afcCnn);
    afcCnn = nullptr;
  }

  CFRelease(bundleIDStr);
  bundleIDStr = nullptr;

  ReleaseTargetDevice(deviceRef);
}

