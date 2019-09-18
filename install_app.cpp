#include "Headers.h"

extern "C" {
 void install_appCommand(Options *options);
}

void install_appCommand(Options *options) {
  const char * appPath = options->GetNextArgument();
  if (appPath == nullptr) {
    fprintf(stderr, "appもしくはipaのパスが指定されていない\n");
    return;
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  CFStringRef appPathString = CFStringCreateWithCString(nullptr, appPath, kCFStringEncodingUTF8);
  CFURLRef urlRef = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, appPathString, kCFURLPOSIXPathStyle, true);

  CFMutableDictionaryRef optionsRef = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(optionsRef, CFSTR("PackageType"), CFSTR("Developer"));

  // PublicStaging へコピー
  uint32_t resultCode = AMDeviceSecureTransferPath(nullptr, deviceRef, urlRef, optionsRef, nullptr, nullptr);
  if (resultCode != 0) {
    fprintf(stderr, "ERR: AMDeviceSecureTransferPath() = 0x%08X\n", resultCode);
  } else {
    resultCode = AMDeviceSecureInstallApplication(nullptr, deviceRef, urlRef, optionsRef, nullptr, nullptr);
    if (resultCode != 0) {
      fprintf(stderr, "ERR: AMDeviceSecureInstallApplication() = 0x%08X\n", resultCode);
    }
  }

  CFRelease(optionsRef);
  CFRelease(urlRef);
  CFRelease(appPathString);

  ReleaseTargetDevice(deviceRef);
}

