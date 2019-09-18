
#include "Headers.h"

extern "C" { void listdeviceCommand(Options *options); }

// 接続中のデバイスのUDID一覧を取得する
void listdeviceCommand(Options *options) {
  CFArrayRef deviceList = AMDCreateDeviceList();
  CFShow(deviceList);
}

