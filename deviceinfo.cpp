
#include "Headers.h"

extern "C" { void deviceinfoCommand(Options *options); }

void printInfo(AMDeviceRef device, const char *header, const char *domain, const char *key) {
  CFStringRef domainStr = nullptr;
  CFStringRef keyStr = nullptr;

  if (domain != nullptr) {
    domainStr = CFStringCreateWithCString(nullptr, domain, kCFStringEncodingUTF8);
  }
  if (key != nullptr) {
    keyStr = CFStringCreateWithCString(nullptr, key, kCFStringEncodingUTF8);
  }
  CFTypeRef valueRef = AMDeviceCopyValue(device, domainStr, keyStr);
  if (valueRef != nullptr) {
    if (CFGetTypeID(valueRef) == CFStringGetTypeID()) {
      char valueBuffer[1024];
      CFStringGetCString((CFStringRef) valueRef, valueBuffer, sizeof(valueBuffer), kCFStringEncodingUTF8);
      printf("%s\t%s\n", header, valueBuffer);
    } else if (CFGetTypeID(valueRef) == CFBooleanGetTypeID()) {
      Boolean value = CFBooleanGetValue((CFBooleanRef) valueRef);
      printf("%s\t%s\n", header, (value ? "True" : "False"));
    } else {
      printf("%s\t??\n", header);
      CFShow(valueRef);
    }
  } else {
    printf("%s\tNULL\n", header);
  }

  if (valueRef != nullptr) {
    CFRelease(valueRef);
    valueRef = nullptr;
  }
  if (keyStr != nullptr) {
    CFRelease(keyStr);
    keyStr = nullptr;
  }
  if (domainStr != nullptr) {
    CFRelease(domainStr);
    domainStr = nullptr;
  }
}

void deviceinfoCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  printInfo(deviceRef, "UDID", nullptr, "UniqueDeviceID");
  printInfo(deviceRef, "name", nullptr, "DeviceName");
  printInfo(deviceRef, "Product", nullptr, "ProductType");
  printInfo(deviceRef, "OSVER", nullptr, "ProductVersion");
  printInfo(deviceRef, "serial", nullptr, "SerialNumber");
  printInfo(deviceRef, "Model", nullptr, "ModelNumber");

  printInfo(deviceRef, "BuildVersion", nullptr, "BuildVersion");
  printInfo(deviceRef, "CPUArchitecture", nullptr, "CPUArchitecture");
  printInfo(deviceRef, "DeveloperStatus", "com.apple.xcode.developerdomain", "DeveloperStatus");
  printInfo(deviceRef, "IsInternal", "com.apple.mobile.internal", "IsInternal");
  printInfo(deviceRef, "BonjourFullServiceName", "com.apple.mobile.wireless_lockdown", "BonjourFullServiceName");
  printInfo(deviceRef, "EnableWifiDebugging", "com.apple.mobile.wireless_lockdown", "EnableWifiDebugging");
  printInfo(deviceRef, "WirelessHosts", "com.apple.xcode.developerdomain", "WirelessHosts");

  printInfo(deviceRef, "DeveloperModeStatus", "com.apple.security.mac.amfi", "DeveloperModeStatus");
  ReleaseTargetDevice(deviceRef);
}

