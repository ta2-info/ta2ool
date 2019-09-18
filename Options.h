
#pragma once

class Options {
private:
  int commandlineArgumentCount;
  const char ** commandlineArgumentValue;
  int currentArgc;
  const char *command;

public:
  Options();
  virtual ~Options();

  bool parseCommandLine(int argc, const char *argv[]);

  const char * GetCommand();

  const char * GetNextArgument();

};

