
#include "Options.h"
#include "MobileDevice.h"

#include <stdbool.h>

extern "C" {
AMDeviceRef GetTargetDevice(Options *options);
void ReleaseTargetDevice(AMDeviceRef device);
void listFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *basePath);
void pullFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *targetPath, const char *localPathOrDir);
void pushFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *localPathOrDir, const char *targetPath);
void removeFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *path);
void renameFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *oldpath, const char *newpath);
void makeDirectory(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *path);
const char * getFilename(const char *filePath);
bool isDirInLocal(const char *pathOrDir);
}

