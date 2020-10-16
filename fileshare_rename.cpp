
#include "Headers.h"

extern "C" { void fileshare_renameCommand(Options *options); }

void fileshare_renameCommand(Options *options) {
  const char * bundleID = options->GetNextArgument();
  if (bundleID == nullptr) {
    fprintf(stderr, "Please indicate BundleID.\n");
    return;
  }

  // 省略不可
  char tmpPath1[1024];
  strcpy(tmpPath1, "Documents/");
  const char *beforePath = options->GetNextArgument();
  if (beforePath == nullptr) {
    fprintf(stderr, "変更前が指定されていない\n");
    return;
  }
  strcat(tmpPath1, beforePath);

  char tmpPath2[1024];
  strcpy(tmpPath2, "Documents/");
  // 省略不可
  const char * afterPath = options->GetNextArgument();
  if (afterPath == nullptr) {
    fprintf(stderr, "変更後が指定されていない\n");
    return;
  }
  strcat(tmpPath2, afterPath);

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
    renameFile(deviceRef, afcCnn, tmpPath1, tmpPath2);
    AFCConnectionClose(afcCnn);
    afcCnn = nullptr;
  }


  CFRelease(myDict);
  myDict = nullptr;

  CFRelease(bundleIDStr);
  bundleIDStr = nullptr;

  ReleaseTargetDevice(deviceRef);
}

