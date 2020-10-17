
#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>

typedef CFTypeRef AFCConnectionRef;
typedef CFTypeRef AFCIteratorRef;
typedef CFTypeRef AMDeviceRef;
typedef CFTypeRef AMDServiceConnectionRef;
typedef CFTypeRef AFCFileDescriptorRef;
typedef CFTypeRef AFCOperationRef;

#ifdef __cplusplus
extern "C" {
#endif

CFArrayRef AMDCreateDeviceList(void);
uint32_t AMDCopyArrayOfDevicesMatchingQuery(CFArrayRef targetListRef, CFDictionaryRef queryRef, CFArrayRef *resultRef);
uint32_t AMDServiceConnectionSend(AMDServiceConnectionRef serviceConnectionRef, const void *, uint32_t length);
uint32_t AMDServiceConnectionReceive(AMDServiceConnectionRef serviceConnectionRef, void *, uint32_t length);

uint32_t AMDeviceConnect(AMDeviceRef deviceRef);
uint32_t AMDeviceStartSession(AMDeviceRef deviceRef);
uint32_t AMDevicePair(AMDeviceRef deviceRef);

CFTypeRef AMDeviceCopyValue(AMDeviceRef deviceRef, CFStringRef domainRef, CFStringRef keyNameRef);

uint32_t AMDeviceLookupApplications(AMDeviceRef deviceRef, CFDictionaryRef optionsRef, CFDictionaryRef *resultDicRef);

uint32_t AMDeviceSecureStartService(AMDeviceRef deviceRef, CFStringRef serviceNameRef, CFDictionaryRef optionsRef, AMDServiceConnectionRef *amdServiceConnection);
int AMDServiceConnectionGetSocket(AMDServiceConnectionRef amdServiceConnectionRef);
uint32_t AFCConnectionOpen(int handle, int32_t timeout, AFCConnectionRef *);

uint32_t AMDeviceCreateHouseArrestService(AMDeviceRef amDeviceRef, CFStringRef bundleIDRef, CFDictionaryRef optionsRef, AFCConnectionRef *ptr);

uint64_t AFCConnectionGetIOTimeout(AFCConnectionRef afcConnectionRef);
uint32_t AFCConnectionProcessOperation(AFCConnectionRef afcConnectionRef, AFCOperationRef operationRef, double timeout);
uint32_t AFCDirectoryOpen(AFCConnectionRef afcConnectionRef, const char* name, AFCIteratorRef *iteratorRef);
uint32_t AFCDirectoryRead(AFCConnectionRef afcConnectionRef, AFCIteratorRef iteratorRef, const char** ptr);
uint32_t AFCDirectoryClose(AFCConnectionRef afcConnectionRef, AFCIteratorRef iteratorRef);
uint32_t AFCDirectoryCreate(AFCConnectionRef afcConnectionRef, const char* name);
uint32_t AFCFileInfoOpen(AFCConnectionRef afcConnectionRef, const char* name, AFCIteratorRef *ptr);
uint32_t AFCKeyValueRead(AFCIteratorRef iteratorRef, const char **keyPtr, const char **valuePtr);
uint32_t AFCKeyValueClose(AFCIteratorRef iteratorRef);
uint32_t AFCConnectionClose(AFCConnectionRef afcConection);
uint32_t AFCFileRefOpen(AFCConnectionRef afcConnectionRef, const char* name, int flags, AFCFileDescriptorRef *ptr);
uint32_t AFCFileRefClose(AFCConnectionRef afcConnectionRef, AFCFileDescriptorRef descRef);
uint32_t AFCFileRefRead(AFCConnectionRef afcConnectionRef, AFCFileDescriptorRef descRef, void *result, long *length);
uint32_t AFCFileRefWrite(AFCConnectionRef afcConnectionRef, AFCFileDescriptorRef descRef, void *result, long length);
uint32_t AFCRemovePath(AFCConnectionRef afcConnectionRef, const char* name);
uint32_t AFCRenamePath(AFCConnectionRef afcConnectionRef, const char *oldpath, const char *newpath);
AFCOperationRef AFCOperationCreateSetModTime(CFAllocatorRef allocator, CFStringRef
 filePath, uint64_t time, void* context);

uint32_t AMDeviceStopSession(AMDeviceRef deviceRef);
uint32_t AMDeviceDisconnect(AMDeviceRef deviceRef);

uint32_t AMDeviceSecureTransferPath(AMDServiceConnectionRef amdServiceConnectionRef, AMDeviceRef deviceRef, CFURLRef targetRef, CFDictionaryRef optionsRef, void* callback, void* callbackArg);
uint32_t AMDeviceSecureInstallApplication(AMDServiceConnectionRef amdServiceConnectionRef, AMDeviceRef amDeviceRef, CFURLRef targetRef, CFDictionaryRef optionsRef, void* callback, void* callbackArg);

uint32_t AMDeviceSecureUninstallApplication(AMDServiceConnectionRef cnn, AMDeviceRef deviceRef, CFStringRef bundleID, CFDictionaryRef options, void* callback, void* callbackArg);

#ifdef __cplusplus
}
#endif

