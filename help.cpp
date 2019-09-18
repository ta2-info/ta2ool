
#include "Headers.h"

extern "C" { void helpCommand(Options *options); }

void helpCommand(Options *options) {
  printf("Commands:\n");
  printf("  help\n");
  printf("  listdevice\n");
  printf("  deviceinfo\n");
  printf("  listapp\n");
  printf("  listappname\n");
  printf("  listappraw\n");
  printf("  listfilesharedapp\n");
  printf("  fileshare_ls bundleID [path]\n");
  printf("  fileshare_pull bundleID pathInDevice [filePathOrDirectoryInLocal]\n");
  printf("  file_ls [path]\n");
  printf("  file_pull pathInDevice [filePathOrDirectoryInLocal]\n");
  printf("  file_rm path\n");
  printf("  app_ls bundleID [path]\n");
  printf("  app_pull bundleID pathInDevice [filePathOrDirectoryInLocal]\n");
  printf("  install_app path_of_ipa_or_app\n");
  printf("  uninstall_app bundleID\n");
  printf("  location latitude longitude\n");
  printf("  location_reset\n");
}

