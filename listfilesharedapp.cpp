
#include "Headers.h"

extern "C" {
  void listfilesharedappCommand(Options *options);
  static void printKeysAndValues(const void* key, const void* value, void* context);
}

static void printKeysAndValues(const void* key, const void* value, void* context) {
  CFDictionaryRef dic = (CFDictionaryRef) value;

  CFBooleanRef fileSharing = (CFBooleanRef) CFDictionaryGetValue(dic, CFSTR("UIFileSharingEnabled"));
  if (fileSharing != nullptr && CFBooleanGetValue(fileSharing)) {
    char bundleID[1024];
    CFStringGetCString((CFStringRef) key, bundleID, sizeof(bundleID), kCFStringEncodingUTF8);
    puts(bundleID);
  }

}

// 「ファイル共有」が有効になっているアプリの検索
void listfilesharedappCommand(Options *options) {
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


