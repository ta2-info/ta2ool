
#include "Headers.h"

extern "C" { void fileshare_mkdirCommand(Options *options); }

void fileshare_mkdirCommand(Options *options) {
  const char * bundleID = options->GetNextArgument();
  if (bundleID == nullptr) {
    fprintf(stderr, "Please indicate BundleID.\n");
    return;
  }

  // 省略不可
  char tmpPath1[1024];
  strcpy(tmpPath1, "Documents/");
  const char *targetPath = options->GetNextArgument();
  if (targetPath == nullptr) {
    fprintf(stderr, "名称が指定されていない\n");
    return;
  }
  strcat(tmpPath1, targetPath);

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
    makeDirectory(deviceRef, afcCnn, tmpPath1);
    AFCConnectionClose(afcCnn);
    afcCnn = nullptr;
  }


  CFRelease(myDict);
  myDict = nullptr;

  CFRelease(bundleIDStr);
  bundleIDStr = nullptr;

  ReleaseTargetDevice(deviceRef);
}

