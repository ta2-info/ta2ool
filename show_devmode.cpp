
#include "Headers.h"

extern "C" {
 void show_devmodeCommand(Options *options);
}


void show_devmodeCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    fprintf(stderr, "Device not found...\n");
    return;
  }

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.amfi.lockdown"), nullptr, &cnn);
  if (resultCode == 0) {
    CFMutableDictionaryRef myDict = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    int zeroValue = 0;
    CFNumberRef zeroRef = CFNumberCreate(nullptr, kCFNumberSInt32Type, &zeroValue);
    CFDictionarySetValue(myDict, CFSTR("action"), zeroRef);
    CFRelease(zeroRef);
    resultCode = AMDServiceConnectionSendMessage(cnn, myDict, kCFPropertyListXMLFormat_v1_0);
    CFRelease(myDict);
    printf("resultCode = %d\n", resultCode);

    CFDictionaryRef result = nullptr;
    CFPropertyListFormat format = kCFPropertyListXMLFormat_v1_0;
    resultCode = AMDServiceConnectionReceiveMessage(cnn, &result, &format);
    printf("resultCode = %d\n", resultCode);
    CFShow(result);
  } else {
    fprintf(stderr, "Cannot start service 'com.apple.amfi.lockdown'.\n");
  }

  ReleaseTargetDevice(deviceRef);
}

