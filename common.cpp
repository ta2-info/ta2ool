
#include "Headers.h"

#include <string.h>
#include <sys/stat.h>

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
    // TODO: ディレクトリのサポート
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
    resultCode = AFCFileRefClose(afcCnn, fileDesc);
    printf("resultCode = %d\n", resultCode);

    
  }
}


/*
 ローカル   リモート   動作
---------------------------------------------
 null       *          エラー!!
 *          null       エラー!!
 file       dir        指定したローカルのファイル名で作成
 他                    まだサポートしていない
*/
void pushFile(AMDeviceRef deviceRef, AFCConnectionRef afcCnn, const char *localPathOrDir, const char *targetPath) {
  if (localPathOrDir == nullptr || targetPath == nullptr) {
    fprintf(stderr, "ERR: Invalid Argument.\n");
    return;
  }

  struct stat statinfo;
  if (stat(localPathOrDir, &statinfo) == -1) {
    fprintf(stderr, "ローカルのパスの指定が間違っている可能性がある\n");
    return;
  }

  if ((statinfo.st_mode & S_IFMT) == S_IFDIR) {
    // TODO: ディレクトリのサポート
    fprintf(stderr, "ディレクトリはまだサポートしていないのでファイルを指定してください。\n");
    return;
  }

  long readSize = statinfo.st_size;
  uint8_t* result = (uint8_t*)malloc(readSize);
  if (result == nullptr) {
    fprintf(stderr, "メモリ確保に失敗しました。ファイルが大きすぎるのでは？\n");
    return;
  }
  int fd = open(localPathOrDir, O_RDONLY);
  long doneSize = 0;
  while (doneSize < readSize) {
    long tmp = read(fd, result + doneSize, (readSize - doneSize));
    if (tmp > 0) {
      doneSize += tmp;
    } else {
      fprintf(stderr, "正常にファイル読み込みをできない\n");
      close(fd);
      free(result);
      return;
    }
  }
  close(fd);

  // 保存先のファイル作成
  AFCFileDescriptorRef fileDesc = nullptr;
  // falg を 1はファイルがない場合は作成してくれないので向かない
  // flag が 2〜7の場合のみファイルがない場合は新規作成する。ただしファイルがある場合の振る舞いが以下のように異なる。
  // flag を 2で作るとファイルを小さくできない(サイズそのままの状態の上書き用途)
  // flag を 3,4,7でopenするとサイズ0スタートになるので切り詰め可能(3,4,7での違いはまだ不明)
  // flag を 5,6にすると追記っぽい(5,6の違いは不明)
  uint32_t resultCode = AFCFileRefOpen(afcCnn, targetPath, 3, &fileDesc);
  if (resultCode == 0) {
    doneSize = 0;
    while (doneSize < readSize) {
      long writeSize = readSize - doneSize;
      resultCode = AFCFileRefWrite(afcCnn, fileDesc, result + doneSize, writeSize);
      if (writeSize > 0) {
        doneSize += writeSize;
      } else {
        fprintf(stderr, "正常にファイルを書き込めない\n");
        AFCFileRefClose(afcCnn, fileDesc);
        free(result);
        return;
      }
    }
    resultCode = AFCFileRefClose(afcCnn, fileDesc);

    // Update timestamp
    setTimestamp(afcCnn, targetPath, statinfo.st_mtime * 1000000000);
  } else {
    printf("ファイルを作成できない\n");
  }
  free(result);

  printf("resultCode = %d\n", resultCode);
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

