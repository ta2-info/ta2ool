
#include "Headers.h"

extern "C" { void app_pushCommand(Options *options); }

void app_pushCommand(Options *options) {
  const char * bundleID = options->GetNextArgument();
  if (bundleID == nullptr) {
    fprintf(stderr, "Please indicate BundleID.\n");
    return;
  }

  // 省略不可
  const char * localPathOrDir = options->GetNextArgument();
  if (localPathOrDir == nullptr) {
    fprintf(stderr, "パスが指定されていない\n");
    return;
  }

  // 省略不可
  const char * targetPath = options->GetNextArgument();
  if (targetPath == nullptr) {
    fprintf(stderr, "パスが指定されていない\n");
    return;
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    fprintf(stderr, "利用可能なデバイスが見つからない\n");
    return;
  }

  CFStringRef bundleIDStr = CFStringCreateWithCString(nullptr, bundleID, kCFStringEncodingUTF8);

  AFCConnectionRef afcCnn = nullptr;
  uint32_t resultCode = AMDeviceCreateHouseArrestService(deviceRef, bundleIDStr, nullptr, &afcCnn);
  if (resultCode != 0) {
    printf("Invlid BundleID or Not supported.(0x%08X)\n", resultCode);
  } else {
    pushFile(deviceRef, afcCnn, localPathOrDir, targetPath);
    AFCConnectionClose(afcCnn);
    afcCnn = nullptr;
  }

  CFRelease(bundleIDStr);
  bundleIDStr = nullptr;

  ReleaseTargetDevice(deviceRef);
}
