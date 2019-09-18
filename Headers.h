
#include "Options.h"
#include "MobileDevice.h"

#include <stdbool.h>

extern "C" {
AMDeviceRef GetTargetDevice(Options *options);
void ReleaseTargetDevice(AMDeviceRef device);
void listFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *basePath);
void pullFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *targetPath, const char *localPathOrDir);
const char * getFilename(const char *filePath);
bool isDirInLocal(const char *pathOrDir);
}

