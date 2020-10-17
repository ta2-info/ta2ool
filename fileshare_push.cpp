
#include "Headers.h"

extern "C" { void fileshare_pushCommand(Options *options); }

void fileshare_pushCommand(Options *options) {
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

  char tmpPath[1024];
  strcpy(tmpPath, "Documents/");
  // 省略可能
  const char * targetPath = options->GetNextArgument();
  if (targetPath != nullptr) {
    strcat(tmpPath, targetPath);
  } else {
    strcat(tmpPath, localPathOrDir);
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    fprintf(stderr, "利用可能なデバイスが見つからない\n");
    return;
  }

  CFStringRef bundleIDStr = CFStringCreateWithCString(nullptr, bundleID, kCFStringEncodingUTF8);

  CFMutableDictionaryRef myDict = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(myDict, CFSTR("Command"), CFSTR("VendDocuments"));
  AFCConnectionRef afcCnn = nullptr;
  uint32_t resultCode = AMDeviceCreateHouseArrestService(deviceRef, bundleIDStr, myDict, &afcCnn);
  if (resultCode != 0) {
    printf("Invlid BundleID or Not supported.\n");
  } else {
    pushFile(deviceRef, afcCnn, localPathOrDir, tmpPath);
    AFCConnectionClose(afcCnn);
    afcCnn = nullptr;
  }


  CFRelease(myDict);
  myDict = nullptr;

  CFRelease(bundleIDStr);
  bundleIDStr = nullptr;

  ReleaseTargetDevice(deviceRef);
}

