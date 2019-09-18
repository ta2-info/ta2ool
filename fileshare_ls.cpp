
#include "Headers.h"

extern "C" { void fileshare_lsCommand(Options *options); }


// 「ファイル共有」機能
void fileshare_lsCommand(Options *options) {
  const char * bundleID = options->GetNextArgument();
  if (bundleID == nullptr) {
    fprintf(stderr, "Please indicate BundleID.\n");
    return;
  }
  const char * basePath = options->GetNextArgument();
  if (basePath == nullptr) {
    basePath = ".";
  }
  char tmpPath[1024];
  strcpy(tmpPath, "Documents/");
  strcat(tmpPath, basePath);

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
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
    listFile(deviceRef, afcCnn, tmpPath);
    AFCConnectionClose(afcCnn);
    afcCnn = nullptr;
  }


  CFRelease(myDict);
  myDict = nullptr;

  CFRelease(bundleIDStr);
  bundleIDStr = nullptr;

  ReleaseTargetDevice(deviceRef);
}

