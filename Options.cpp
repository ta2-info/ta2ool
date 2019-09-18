
// コマンドラインオプションに対するルール
//
// 「コマンド」は１つだけ指定可能
// 「コマンド」を指定していない場合は help コマンド指定と同等とする
// 「オプション」は「コマンド」よりも前に指定する
// 「オプション」はハイフンから始める
// 「オプション」で「パラメータ」が必要な場合は「コマンド」の次以降に記述する

#include "Options.h"
#include <stdio.h>

Options::Options() : command("help") {
}

Options::~Options() {
}

bool Options::parseCommandLine(int argc, const char *argv[]) {
  this->commandlineArgumentCount = argc;
  this->commandlineArgumentValue = argv;

  if (argc < 2) {
    // helpで確定
    return true;
  }

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      // Options
      printf("Option!!\n");
    } else {
      // Command
      this->command = argv[i];
      this->currentArgc = i + 1;
      break;
    }
  }

  return true;
}

const char * Options::GetCommand() {
  return this->command;
}

const char * Options::GetNextArgument() {
  if (this->currentArgc < this->commandlineArgumentCount) {
    return this->commandlineArgumentValue[this->currentArgc++];
  } else {
    return nullptr;
  }
}

