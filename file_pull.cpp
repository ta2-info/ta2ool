
#include "Headers.h"

#include <sys/stat.h>

extern "C" { void file_pullCommand(Options *options); }

void file_pullCommand(Options *options) {
  const char * targetPath = options->GetNextArgument();
  if (targetPath == nullptr) {
    fprintf(stderr, "パスが指定されていない\n");
    return;
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  // 省略した場合は nullptr
  const char * localPathOrDir = options->GetNextArgument();

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.afc"), nullptr, &cnn);
  if (resultCode == 0) {
    int socket = AMDServiceConnectionGetSocket(cnn);
    AFCConnectionRef afcCnn = nullptr;
    resultCode = AFCConnectionOpen(socket, 0, &afcCnn);
    if (resultCode == 0) {
      pullFile(deviceRef, afcCnn, targetPath, localPathOrDir);
      AFCConnectionClose(afcCnn);
    }
    close(socket);
  }

  ReleaseTargetDevice(deviceRef);
}

