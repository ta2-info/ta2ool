
#include "Headers.h"

#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

void setTimestamp(AFCConnectionRef afcCnn, const char *filePath, uint64_t timestamp) {
  CFAllocatorRef allocatorRef = CFGetAllocator(afcCnn);

  CFStringRef filePathRef = CFStringCreateWithCString(nullptr, filePath, kCFStringEncodingUTF8);
  AFCOperationRef operationRef = AFCOperationCreateSetModTime(allocatorRef, filePathRef, timestamp, nullptr);
  CFRelease(filePathRef);

  double timeout = AFCConnectionGetIOTimeout(afcCnn);
  AFCConnectionProcessOperation(afcCnn, operationRef, timeout);
  CFRelease(operationRef);
  CFRelease(allocatorRef);
}



// resultCode
// 7: パラメータエラー的なやつ?
// 8: 存在しない的なやつ?
// 10: 権限的なやつ?

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


// ローカル側のエラーを表す resultCode
static const uint32_t kLocalError = 0xFFFFFFFF;

// 転送時のバッファサイズ
static const long kTransferBufferSize = 1024 * 1024;

// 末尾の '/' を取り除いたパスを作る（"/" だけの場合はそのまま）
static void trimTrailingSlash(char *dst, size_t dstSize, const char *src) {
  snprintf(dst, dstSize, "%s", src);
  size_t len = strlen(dst);
  while (len > 1 && dst[len - 1] == '/') {
    dst[--len] = '\0';
  }
}

// リモートのファイル情報を取得する
static uint32_t getRemoteFileInfo(AFCConnectionRef afcCnn, const char *path, bool *isDir, long *fileSize, long *fileTimespec) {
  AFCIteratorRef iterator = nullptr;
  uint32_t resultCode = AFCFileInfoOpen(afcCnn, path, &iterator);
  if (resultCode != 0) {
    return resultCode;
  }
  if (iterator == nullptr) {
    return 8;
  }

  *isDir = false;
  *fileSize = 0;
  *fileTimespec = 0;
  while (1) {
    const char* key = nullptr;
    const char* value = nullptr;
    AFCKeyValueRead(iterator, &key, &value);
    if (key == nullptr) {
      break;
    }
    if (strcmp(key, "st_size") == 0) {
      *fileSize = atol(value);
    } else if (strcmp(key, "st_mtime") == 0) {
      *fileTimespec = atol(value);
    } else if (strcmp("st_ifmt", key) == 0) {
      if (strcmp("S_IFDIR", value) == 0) {
        *isDir = true;
      }
    }
  }
  AFCKeyValueClose(iterator);
  return 0;
}

// リモートのファイル1つをローカルのパスへコピーする
static uint32_t pullSingleFile(AFCConnectionRef afcCnn, const char *targetPath, const char *outputFilePath, long fileTimespec) {
  AFCFileDescriptorRef fileDesc = nullptr;
  uint32_t resultCode = AFCFileRefOpen(afcCnn, targetPath, 1, &fileDesc);
  if (resultCode != 0) {
    return resultCode;
  }

  int fd = open(outputFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd == -1) {
    fprintf(stderr, "ローカルにファイルを作成できない (%s)\n", outputFilePath);
    AFCFileRefClose(afcCnn, fileDesc);
    return kLocalError;
  }

  void *buffer = malloc(kTransferBufferSize);
  while (1) {
    long readSize = kTransferBufferSize;
    resultCode = AFCFileRefRead(afcCnn, fileDesc, buffer, &readSize);
    if (resultCode != 0 || readSize <= 0) {
      break;
    }
    if (write(fd, buffer, readSize) != readSize) {
      fprintf(stderr, "ローカルへの書き込みに失敗した (%s)\n", outputFilePath);
      resultCode = kLocalError;
      break;
    }
  }
  free(buffer);

  // st_mtime はナノ秒単位
  struct timespec times[2];
  times[0].tv_sec = fileTimespec / 1000000000;
  times[0].tv_nsec = fileTimespec % 1000000000;
  times[1] = times[0];
  futimens(fd, times);
  close(fd);

  uint32_t closeResultCode = AFCFileRefClose(afcCnn, fileDesc);
  return (resultCode != 0) ? resultCode : closeResultCode;
}

static void pullDirectory(AFCConnectionRef afcCnn, const char *targetDir, const char *localDir);

// リモートのファイル or ディレクトリをローカルのパスへコピーする
static void pullEntry(AFCConnectionRef afcCnn, const char *targetPath, const char *localPath) {
  bool isDir = false;
  long fileSize = 0;
  long fileTimespec = 0;
  uint32_t resultCode = getRemoteFileInfo(afcCnn, targetPath, &isDir, &fileSize, &fileTimespec);
  if (resultCode != 0) {
    fprintf(stderr, "ファイルの情報を取得できませんでした。(0x%08X) %s\n", resultCode, targetPath);
    return;
  }

  if (isDir) {
    pullDirectory(afcCnn, targetPath, localPath);
  } else {
    resultCode = pullSingleFile(afcCnn, targetPath, localPath, fileTimespec);
    if (resultCode == 0) {
      printf("%s -> %s\n", targetPath, localPath);
    } else {
      fprintf(stderr, "コピーに失敗しました。(0x%08X) %s\n", resultCode, targetPath);
    }
  }
}

// リモートのディレクトリ以下をローカルのディレクトリへ再帰でコピーする
static void pullDirectory(AFCConnectionRef afcCnn, const char *targetDir, const char *localDir) {
  if (mkdir(localDir, 0777) == -1 && (errno != EEXIST || isDirInLocal(localDir) == false)) {
    fprintf(stderr, "ローカルにディレクトリを作成できない (%s)\n", localDir);
    return;
  }

  AFCIteratorRef dirIterator = nullptr;
  uint32_t resultCode = AFCDirectoryOpen(afcCnn, targetDir, &dirIterator);
  if (resultCode != 0 || dirIterator == nullptr) {
    fprintf(stderr, "ディレクトリを開けませんでした。(0x%08X) %s\n", resultCode, targetDir);
    return;
  }

  while (1) {
    const char *name = nullptr;
    AFCDirectoryRead(afcCnn, dirIterator, &name);
    if (name == nullptr) {
      break;
    }
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
      continue;
    }

    char childTargetPath[1024];
    char childLocalPath[1024];
    snprintf(childTargetPath, sizeof(childTargetPath), "%s/%s", targetDir, name);
    snprintf(childLocalPath, sizeof(childLocalPath), "%s/%s", localDir, name);
    pullEntry(afcCnn, childTargetPath, childLocalPath);
  }
  AFCDirectoryClose(afcCnn, dirIterator);
}


/*
 リモート   ローカル   動作
---------------------------------------------
 file       null       リモートのファイル名で作成
 file       path       ローカルで指定したファイル名で作成
 file       dir        ローカルのディレクトリにリモートのファイル名で作成
 dir        null       カレントディレクトリにリモートのディレクトリ名で作成し、以下を再帰で再現
 dir        path       ローカルで指定したパスにディレクトリを作成し、リモートのディレクトリ以下を再帰で再現
 dir        dir        ローカルで指定したディレクトリ以下にリモートのディレクトリ名で作成し、以下を再帰で再現
*/
void pullFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *targetPath, const char *localPathOrDir) {
  char remotePath[1024];
  trimTrailingSlash(remotePath, sizeof(remotePath), targetPath);

  bool isDir = false;
  long fileSize = 0;
  long fileTimespec = 0;
  uint32_t resultCode = getRemoteFileInfo(afcCnn, remotePath, &isDir, &fileSize, &fileTimespec);
  if (resultCode != 0) {
    // 権限的な場合は 7 を返す
    fprintf(stderr, "ファイルの情報を取得できませんでした。(0x%08X)\n", resultCode);
    return;
  }

  const char *filename = getFilename(remotePath);
  if (filename[0] == '\0') {
    filename = ".";
  }

  char outputPath[1024];
  if (localPathOrDir == nullptr) {
    // リモートのファイル名をそのまま使う
    snprintf(outputPath, sizeof(outputPath), "%s", filename);
  } else if (isDirInLocal(localPathOrDir)) {
    // 出力先ディレクトリを指定しつつ、リモートのファイル名をそのまま使う
    snprintf(outputPath, sizeof(outputPath), "%s/%s", localPathOrDir, filename);
  } else {
    // リモートのファイル名は使わず指定されたパスへ出力
    snprintf(outputPath, sizeof(outputPath), "%s", localPathOrDir);
  }

  if (isDir) {
    pullDirectory(afcCnn, remotePath, outputPath);
  } else {
    resultCode = pullSingleFile(afcCnn, remotePath, outputPath, fileTimespec);
    printf("resultCode = %d\n", resultCode);
  }
}


// ローカルのファイル1つをリモートのパスへコピーする
static uint32_t pushSingleFile(AFCConnectionRef afcCnn, const char *localPath, const char *targetPath, time_t mtime) {
  int fd = open(localPath, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "ローカルのファイルを開けない (%s)\n", localPath);
    return kLocalError;
  }

  // 保存先のファイル作成
  AFCFileDescriptorRef fileDesc = nullptr;
  // falg を 1はファイルがない場合は作成してくれないので向かない
  // flag が 2〜7の場合のみファイルがない場合は新規作成する。ただしファイルがある場合の振る舞いが以下のように異なる。
  // flag を 2で作るとファイルを小さくできない(サイズそのままの状態の上書き用途)
  // flag を 3,4,7でopenするとサイズ0スタートになるので切り詰め可能(3,4,7での違いはまだ不明)
  // flag を 5,6にすると追記っぽい(5,6の違いは不明)
  uint32_t resultCode = AFCFileRefOpen(afcCnn, targetPath, 3, &fileDesc);
  if (resultCode != 0) {
    fprintf(stderr, "ファイルを作成できない (%s)\n", targetPath);
    close(fd);
    return resultCode;
  }

  uint8_t *buffer = (uint8_t *)malloc(kTransferBufferSize);
  while (1) {
    long readSize = read(fd, buffer, kTransferBufferSize);
    if (readSize == 0) {
      break;
    }
    if (readSize < 0) {
      fprintf(stderr, "正常にファイル読み込みをできない (%s)\n", localPath);
      resultCode = kLocalError;
      break;
    }
    resultCode = AFCFileRefWrite(afcCnn, fileDesc, buffer, readSize);
    if (resultCode != 0) {
      fprintf(stderr, "正常にファイルを書き込めない (%s)\n", targetPath);
      break;
    }
  }
  free(buffer);
  close(fd);

  uint32_t closeResultCode = AFCFileRefClose(afcCnn, fileDesc);
  if (resultCode == 0) {
    resultCode = closeResultCode;
  }

  // Update timestamp
  if (resultCode == 0) {
    setTimestamp(afcCnn, targetPath, mtime * 1000000000);
  }
  return resultCode;
}

// ローカルのディレクトリ以下をリモートのディレクトリへ再帰でコピーする
static void pushDirectory(AFCConnectionRef afcCnn, const char *localDir, const char *targetDir) {
  uint32_t resultCode = AFCDirectoryCreate(afcCnn, targetDir);
  if (resultCode != 0) {
    // 既にディレクトリが存在する場合はそのまま使う
    bool isDir = false;
    long fileSize = 0;
    long fileTimespec = 0;
    if (getRemoteFileInfo(afcCnn, targetDir, &isDir, &fileSize, &fileTimespec) != 0 || isDir == false) {
      fprintf(stderr, "ディレクトリを作成できない (0x%08X) %s\n", resultCode, targetDir);
      return;
    }
  }

  DIR *dir = opendir(localDir);
  if (dir == nullptr) {
    fprintf(stderr, "ローカルのディレクトリを開けない (%s)\n", localDir);
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char childLocalPath[1024];
    char childTargetPath[1024];
    snprintf(childLocalPath, sizeof(childLocalPath), "%s/%s", localDir, entry->d_name);
    snprintf(childTargetPath, sizeof(childTargetPath), "%s/%s", targetDir, entry->d_name);

    struct stat statinfo;
    if (stat(childLocalPath, &statinfo) == -1) {
      fprintf(stderr, "ローカルのファイル情報を取得できない (%s)\n", childLocalPath);
      continue;
    }
    if ((statinfo.st_mode & S_IFMT) == S_IFDIR) {
      pushDirectory(afcCnn, childLocalPath, childTargetPath);
    } else if ((statinfo.st_mode & S_IFMT) == S_IFREG) {
      resultCode = pushSingleFile(afcCnn, childLocalPath, childTargetPath, statinfo.st_mtime);
      if (resultCode == 0) {
        printf("%s -> %s\n", childLocalPath, childTargetPath);
      } else {
        fprintf(stderr, "コピーに失敗しました。(0x%08X) %s\n", resultCode, childLocalPath);
      }
    }
  }
  closedir(dir);
}


/*
 ローカル   リモート   動作
---------------------------------------------
 null       *          エラー!!
 *          null       エラー!!
 file       path       リモートで指定したパスにファイルを作成
 dir        path       リモートで指定したパスにディレクトリを作成し、ローカルのディレクトリ以下を再帰で再現
                       (既にディレクトリが存在する場合はその中に上書きコピー)
*/
void pushFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *localPathOrDir, const char *targetPath) {
  if (localPathOrDir == nullptr || targetPath == nullptr) {
    fprintf(stderr, "ERR: Invalid Argument.\n");
    return;
  }

  char localPath[1024];
  char remotePath[1024];
  trimTrailingSlash(localPath, sizeof(localPath), localPathOrDir);
  trimTrailingSlash(remotePath, sizeof(remotePath), targetPath);

  struct stat statinfo;
  if (stat(localPath, &statinfo) == -1) {
    fprintf(stderr, "ローカルのパスの指定が間違っている可能性がある\n");
    return;
  }

  if ((statinfo.st_mode & S_IFMT) == S_IFDIR) {
    pushDirectory(afcCnn, localPath, remotePath);
  } else {
    uint32_t resultCode = pushSingleFile(afcCnn, localPath, remotePath, statinfo.st_mtime);
    printf("resultCode = %d\n", resultCode);
  }
}

void removeFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *path) {
  uint32_t resultCode;
  AFCIteratorRef iterator;
  resultCode = AFCFileInfoOpen(afcCnn, path, &iterator);
  if (resultCode != 0 || iterator == nullptr) {
    // 権限的な場合は 7 を返す
    // 存在しない場合は 8 を返す
    fprintf(stderr, "ファイルの情報を取得できませんでした。(0x%08X)\n", resultCode);
    return;
  }

  bool isDir = false;
  while (1) {
    const char* key = nullptr;
    const char* value = nullptr;
    AFCKeyValueRead(iterator, &key, &value);
    if (key == nullptr) {
      break;
    }
    if ((strcmp("st_ifmt", key) == 0) && (strcmp("S_IFDIR", value) == 0)) {
      isDir = true;
    }
  }
  AFCKeyValueClose(iterator);

  if (isDir == false) {
    // ファイルの場合
    resultCode = AFCRemovePath(afcCnn, path);
    if (resultCode == 0) {
      printf("削除しました\n");
    } else {
      printf("resultCode = %d\n", resultCode);
    }
  } else {
    // ディレクトリの場合
    resultCode = AFCRemovePath(afcCnn, path);
    if (resultCode == 0) {
      printf("削除しました\n");
    } else {
      printf("resultCode = %d\n", resultCode);
if (resultCode == 1) {
  fprintf(stderr, "※ディレクトリ内が空ではない時はエラーコード 1 を返却するようだ\n");
}

// TODO: 強制的に消す仕組みを用意したい （例えば ディレクトリ名/* と指定することでディレクトリ配下は消えるとか）

    }
  }

}


void renameFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *oldpath, const char *newpath) {
  uint32_t resultCode;
  resultCode = AFCRenamePath(afcCnn, oldpath, newpath);
  if (resultCode != 0) {
    printf("resultCode = %d\n", resultCode);
  }
}

void makeDirectory(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *path) {
  uint32_t resultCode;
  resultCode = AFCDirectoryCreate(afcCnn, path);
  printf("resultCode = %d\n", resultCode);

  // 0: 成功
  // 10: 権限拒否?(セキュリティ的理由のほう)
  // 16: 指定された名称のファイルが既に存在する
}

