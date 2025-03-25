#include "kernel/types.h"
#include "user/user.h"

static int using_synchronisation;
static int mtx;

void print_args(int argc, char** argv, int using_synchronisation) {
  for (int i = 0; i < argc; ++i) {
    char s[2] = {'\0', '\0'};
    for (char* c = argv[i]; *c != '\0'; ++c) {
      s[0] = *c;
      if (using_synchronisation) {
        if (mutex_lock(mtx)) {
          fprintf(2, "mutexsync: failed to lock the mutex.\n");
          exit(1);
        }
      }
      printf("pid: arg i, char \'%s\'\n", s);
      if (using_synchronisation) {
        if (mutex_unlock(mtx)) {
          fprintf(2, "mutexsync: failed to unlock the mutex.\n");
          exit(1);
        }
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    using_synchronisation = 0;
  } else if (argc == 2 && !strcmp(argv[1], "--sync")) {
    using_synchronisation = 1;
  } else {
    fprintf(2, "usage: mutexsync [--sync]\n");
    exit(1);
  }

  if (using_synchronisation) {
    if(mutex(&mtx)){
      fprintf(2, "mutexsync: failed to create a mutex.\n");
      exit(1);
    }
  }

  int pid = fork();

  if (pid < 0) {
    fprintf(2, "mutexsync: failed to create a child process.\n");
    exit(1);

  } else if (pid == 0) {
    print_args(argc, argv, using_synchronisation);
  } else {
    print_args(argc, argv, using_synchronisation);
    wait(0);
  }

  if (using_synchronisation) {
    if(close(mtx)){
      fprintf(2, "mutexsync: failed to close the mutex.\n");
      exit(1);
    }
  }

  exit(0);
}