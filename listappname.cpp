
#include "Headers.h"

extern "C" {
  void listappnameCommand(Options *options);
  static void printKeysAndValues(const void* key, const void* value, void* context);
}

static void printKeysAndValues(const void* key, const void* value, void* context) {
  char bundleID[1024];
  CFStringGetCString((CFStringRef) key, bundleID, sizeof(bundleID), kCFStringEncodingUTF8);

  char name[1024];
  CFDictionaryRef dic = (CFDictionaryRef)value;
  CFStringRef nameStr = (CFStringRef) CFDictionaryGetValue(dic, CFSTR("CFBundleDisplayName"));
  if (nameStr != nullptr) {
    CFStringGetCString(nameStr, name, sizeof(name), kCFStringEncodingUTF8);
    printf("%s\t%s\n", name, bundleID);
  }
}

void listappnameCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  CFDictionaryRef result = nullptr;
  uint32_t resultCode = AMDeviceLookupApplications(deviceRef, nullptr, &result);
  if (resultCode == 0 && result != nullptr) {
    CFDictionaryApplyFunction(result, printKeysAndValues, nullptr);
  }

  if (result != nullptr) {
    CFRelease(result);
    result = nullptr;
  }

  ReleaseTargetDevice(deviceRef);
}

