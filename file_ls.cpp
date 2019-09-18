
#include "Headers.h"

extern "C" { void file_lsCommand(Options *options); }

void file_lsCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  const char * basePath = options->GetNextArgument();
  if (basePath == nullptr) {
    basePath = ".";
  }

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.afc"), nullptr, &cnn);
  if (resultCode == 0) {
    int socket = AMDServiceConnectionGetSocket(cnn);
    AFCConnectionRef afcCnn = nullptr;
    resultCode = AFCConnectionOpen(socket, 0, &afcCnn);
    if (resultCode == 0) {
      listFile(deviceRef, afcCnn, basePath);
      AFCConnectionClose(afcCnn);
    }
    close(socket);
  }

  ReleaseTargetDevice(deviceRef);
}

