
#include "Headers.h"

#include <sys/stat.h>

extern "C" { void file_pushCommand(Options *options); }

void file_pushCommand(Options *options) {
  // 省略不可
  const char * localPathOrDir = options->GetNextArgument();
  if (localPathOrDir == nullptr) {
    fprintf(stderr, "パスが指定されていない\n");
    return;
  }

  // 省略不可
  const char * targetPath = options->GetNextArgument();
  if (targetPath == nullptr) {
    fprintf(stderr, "パスが指定されていない\n");
    return;
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    fprintf(stderr, "利用可能なデバイスが見つからない\n");
    return;
  }

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.afc"), nullptr, &cnn);
  if (resultCode == 0) {
    int socket = AMDServiceConnectionGetSocket(cnn);
    AFCConnectionRef afcCnn = nullptr;
    resultCode = AFCConnectionOpen(socket, 0, &afcCnn);
    if (resultCode == 0) {
      pushFile(deviceRef, afcCnn, localPathOrDir, targetPath);
      AFCConnectionClose(afcCnn);
    }
    close(socket);
  }

  ReleaseTargetDevice(deviceRef);
}

