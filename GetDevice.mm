
#include "Headers.h"

#import <Foundation/Foundation.h>

// デバイスを特定する
AMDeviceRef GetTargetDevice(Options *options) {
  // USB接続されているデバイス数が1であれば確定する
  NSDictionary *dic = @{@"MatchingMode": @"MatchAll", @"MatchConnectionType": @"MatchConnectionTypeUSB"};
  CFArrayRef deviceList = nullptr;
  uint32_t resultCode = AMDCopyArrayOfDevicesMatchingQuery(nil, (__bridge CFDictionaryRef)(dic), &deviceList);
  if (resultCode != 0 || deviceList == nullptr) {
    return nullptr;
  }

  CFIndex count = CFArrayGetCount(deviceList);
  if (count != 1) {
    return nullptr;
  }

  AMDeviceRef device = (AMDeviceRef)CFArrayGetValueAtIndex(deviceList, 0);

  AMDeviceConnect(device);
  resultCode = AMDeviceStartSession(device);
  if (resultCode == 0xe8000025 || resultCode == 0xe800001c) {
    // 未ペアリング or 信頼されていない
    AMDevicePair(device);
    return nullptr;
  }

  return device;
}

void ReleaseTargetDevice(AMDeviceRef device) {
  AMDeviceStopSession(device);
  AMDeviceDisconnect(device);
}

