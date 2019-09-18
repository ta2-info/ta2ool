
#include <arpa/inet.h>

#include "Headers.h"

extern "C" {
 void locationCommand(Options *options);
}


void locationCommand(Options *options) {
  AMDeviceRef deviceRef = GetTargetDevice(options);
  if (deviceRef == nullptr) {
    return;
  }

  const char * latitude = options->GetNextArgument();
  if (latitude == nullptr) {
    fprintf(stderr, "Please indicate latitude and longitude in degree.\n");
    return;
  }

  const char * longitude = options->GetNextArgument();
  if (longitude == nullptr) {
    fprintf(stderr, "Please indicate latitude and longitude in degree.\n");
    return;
  }

  AMDServiceConnectionRef cnn = nullptr;
  uint32_t resultCode = AMDeviceSecureStartService(deviceRef, CFSTR("com.apple.dt.simulatelocation"), nullptr, &cnn);
  if (resultCode == 0) {
    uint32_t startMessage = htonl(0);
    uint32_t lat_len = htonl(strlen(latitude));
    uint32_t lng_len = htonl(strlen(longitude));
    AMDServiceConnectionSend(cnn, &startMessage, sizeof(startMessage));
    AMDServiceConnectionSend(cnn, &lat_len, sizeof(lat_len));
    AMDServiceConnectionSend(cnn, latitude, strlen(latitude));
    AMDServiceConnectionSend(cnn, &lng_len, sizeof(lng_len));
    AMDServiceConnectionSend(cnn, longitude, strlen(longitude));
  } else {
    fprintf(stderr, "Cannot start service 'com.apple.dt.simulatelocation'.\n");
  }

  

  ReleaseTargetDevice(deviceRef);
}

