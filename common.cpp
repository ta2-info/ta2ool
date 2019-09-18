
#include "Headers.h"

#include <string.h>
#include <sys/stat.h>

const char * getFilename(const char *filePath) {
  int len = strlen(filePath);
  for (int i = len - 1; i >= 0; i--) {
    if (filePath[i] == '/') {
      return filePath + i + 1;
    }
  }
  return filePath;
}


bool isDirInLocal(const char *pathOrDir) {
  struct stat buf;
  if (stat(pathOrDir, &buf) == -1) {
    return false;
  }
  if (buf.st_mode & S_IFDIR) {
    return true;
  }
  return false;
}


void listFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *basePath) {
  uint32_t resultCode;

  AFCIteratorRef dirIterator = nullptr;
  char tmpPath[1024];
  sprintf(tmpPath, "./%s", basePath);
  resultCode = AFCDirectoryOpen(afcCnn, tmpPath, &dirIterator);

  const char *name = nullptr;
  while (1) {
    AFCDirectoryRead(afcCnn, dirIterator, &name);
    if (name == nullptr) {
      break;
    }

    CFTypeRef fileInfo = nullptr;
    char filepath[1024];
    strcpy(filepath, tmpPath);
    strcat(filepath, "/");
    strcat(filepath, name);

    resultCode = AFCFileInfoOpen(afcCnn, filepath, &fileInfo);

    char fileType = '?';
    char timestamp[30];
    char size[10];
    memset(timestamp, 0x0, sizeof(timestamp));
    memset(size, 0x0, sizeof(size));

    if (fileInfo != nullptr) {
      const char* key = nullptr;
      const char* value = nullptr;
      while (1) {
        AFCKeyValueRead(fileInfo, &key, &value);
        if (key == nullptr) {
          break;
        }
        if (strcmp("st_ifmt", key) == 0) {
          if (strcmp("S_IFDIR", value) == 0) {
            fileType = 'd';
          } else {
            fileType = '-';
          }
          continue;
        }
        if (strcmp("st_size", key) == 0) {
          sprintf(size, "%s", value);
          continue;
        }
        if (strcmp("st_mtime", key) == 0) {
          long timevalue = atol(value) / 1000000000;
          sprintf(timestamp, "%s", asctime(localtime(&timevalue)));
          timestamp[strlen(timestamp) - 1] = '\0';
          continue;
        }
      }
    }
    printf("%c %8s %25s  %s\n", fileType, size, timestamp, name);
  }
}


/*
 リモート   ローカル   動作
---------------------------------------------
 file       null       リモートのファイル名で作成
 file       path       ローカルで指定したファイル名で作成
 file       dir        ローカルのディレクトリにリモートのファイル名で作成 
 dir        null       未サポート）リモートのディレクトリ以下を再帰で再現
 dir        path       未サポート）エラー！！
 dir        dir        未サポート）ローカルで指定したディレクトリ以下にリモートのディレクトリ以下を再帰で再現
*/
void pullFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *targetPath, const char *localPathOrDir) {
  uint32_t resultCode;

  long fileSize = 0;
  long fileTimespec = 0;
  bool isDir = false;

  const char *filename = getFilename(targetPath);

  AFCIteratorRef iterator;
  resultCode = AFCFileInfoOpen(afcCnn, targetPath, &iterator);
  if (resultCode != 0 || iterator == nullptr) {
    // 権限的な場合は 7 を返す
    fprintf(stderr, "ファイルの情報を取得できませんでした。(0x%08X)\n", resultCode);
    return;
  }

  while (1) {
    const char* key = nullptr;
    const char* value = nullptr;
    AFCKeyValueRead(iterator, &key, &value);
    if (key == nullptr) {
      break;
    }
    if (strcmp(key, "st_size") == 0) {
      fileSize = atol(value);
    } else if (strcmp(key, "st_mtime") == 0) {
      fileTimespec = atol(value);
    } else if (strcmp("st_ifmt", key) == 0) {
      if (strcmp("S_IFDIR", value) == 0) {
        isDir = true;
      }
    }
  }
  AFCKeyValueClose(iterator);

  if (isDir) {
    fprintf(stderr, "ディレクトリはまだサポートしていないのでファイルを指定してください。\n");
    return;
  }

  AFCFileDescriptorRef fileDesc = nullptr;
  resultCode = AFCFileRefOpen(afcCnn, targetPath, 1, &fileDesc);
  if (resultCode == 0) {
    void* result = malloc(fileSize);
    long readSize = fileSize;
    resultCode = AFCFileRefRead(afcCnn, fileDesc, result, &readSize);

    if (resultCode == 0) {
      char outputFilePath[1024];
      if (localPathOrDir == nullptr) {
        // リモートのファイル名をそのまま使う
        strcpy(outputFilePath, filename);
      } else if (isDirInLocal(localPathOrDir)) {
        // 出力先ディレクトリを指定しつつ、リモートのファイル名をそのまま使う
        strcpy(outputFilePath, localPathOrDir);
        strcat(outputFilePath, "/");
        strcat(outputFilePath, filename);
      } else {
        // リモートのファイル名は使わず指定されたパスへ出力
        strcpy(outputFilePath, localPathOrDir);
      }
      int fd = open(outputFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      write(fd, result, readSize);
      struct timespec times[2];
      times[0].tv_sec = 0;
      times[0].tv_nsec = fileTimespec;
      times[1].tv_sec = 0;
      times[1].tv_nsec = fileTimespec;
      futimens(fd, times);
      close(fd);
    }

  }


  resultCode = AFCFileRefClose(afcCnn, fileDesc);
  printf("resultCode = %d\n", resultCode);
}

