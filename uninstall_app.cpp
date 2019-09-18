#include "Headers.h"

extern "C" {
 void uninstall_appCommand(Options *options);
}

void uninstall_appCommand(Options *options) {
  const char * bundleName = options->GetNextArgument();
  if (bundleName == nullptr) {
    fprintf(stderr, "BundleIDが指定されていない\n");
    return;
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  CFStringRef bundleIDString = CFStringCreateWithCString(nullptr, bundleName, kCFStringEncodingUTF8);

  uint32_t resultCode = AMDeviceSecureUninstallApplication(nullptr, deviceRef, bundleIDString, nullptr, nullptr, nullptr);
  if (resultCode != 0) {
    fprintf(stderr, "ERR: AMDeviceSecureUninstallApplication() = 0x%08X\n", resultCode);
  }

  CFRelease(bundleIDString);

  ReleaseTargetDevice(deviceRef);
}



