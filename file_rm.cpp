
#include "Headers.h"

extern "C" { void file_rmCommand(Options *options); }

static void mainJob(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *targetPath) {
  uint32_t resultCode;

  AFCIteratorRef iterator = nullptr;
  AFCFileInfoOpen(afcCnn, targetPath, &iterator);
  if (iterator == nullptr) {
    fprintf(stderr, "指定されたファイルの情報を取得できませんでした。\n");
    return;
  }
  AFCKeyValueClose(iterator);

  resultCode = AFCRemovePath(afcCnn, targetPath);
  if (resultCode == 0) {
    printf("削除しました\n");
  } else {
    printf("resultCode = %d\n", resultCode);
  }
}


void file_rmCommand(Options *options) {
  const char * targetPath = options->GetNextArgument();
  if (targetPath == nullptr) {
    fprintf(stderr, "パスが指定されていない\n");
    return;
  }

  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.afc"), nullptr, &cnn);
  if (resultCode == 0) {
    int socket = AMDServiceConnectionGetSocket(cnn);
    AFCConnectionRef afcCnn = nullptr;
    resultCode = AFCConnectionOpen(socket, 0, &afcCnn);
    if (resultCode == 0) {
      mainJob(deviceRef, afcCnn, targetPath);
      AFCConnectionClose(afcCnn);
    }
    close(socket);
  }

  ReleaseTargetDevice(deviceRef);
}

