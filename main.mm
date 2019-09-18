
#include <dlfcn.h>
#include <pthread.h>

#include "Headers.h"

extern "C" { static void* mainJobThread(void* arg); }

static void* mainJobThread(void* arg) {
  @autoreleasepool {
    Options *options = (Options *)arg;

    char commandFunctionName[1024];
    strcpy(commandFunctionName, options->GetCommand());
    strcat(commandFunctionName, "Command");

    void (*commandFunction)(Options *) = (void (*)(Options *))dlsym(RTLD_SELF, commandFunctionName);
    if (commandFunction == nullptr) {
      printf("Invalid Command...\n");
    } else {
      commandFunction(options);
    }

    CFRunLoopRef runLoop = CFRunLoopGetMain();
    while (CFRunLoopIsWaiting(runLoop) == false);
    CFRunLoopStop(runLoop);
  }
  return 0;
}

int main(int argc, const char **argv) {
  @autoreleasepool {
    Options *options = new Options();
    if (options->parseCommandLine(argc, argv) == false) {
      perror("Invalid Command list options.");
      return -1;
    }

    pthread_t tid;
    pthread_create(&tid, nullptr, mainJobThread, options);

    CFRunLoopRun();

    delete options;
    options = nullptr;
  }

  return 0;
}

