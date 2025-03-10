#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char **argv) {
  int is_scenario_kill;
  if (argc == 1) {
    is_scenario_kill = 0;
  } else if (argc == 2 && !strcmp(argv[1], "--kill")) {
    is_scenario_kill = 1;
  } else {
    fprintf(2, "usage: childps [--kill]\n");
    exit(2);
  }

  int pid = fork();

  if (pid < 0) {
    fprintf(2, "childps: failed to create a child process.\n");
    exit(2);

  } else if (pid == 0) {
    sleep(100);
    exit(1);

  } else {
    printf("pid = %d, child pid = %d\n", getpid(), pid);
    if (is_scenario_kill) {
      if(kill(pid)) {
        fprintf(2, "childps: failed to kill child process (pid=%d)\n", pid);
      }
    }

    int exit_code;
    int child_pid = wait(&exit_code);
    printf("child pid = %d, exit code: %d\n", child_pid, exit_code);
  }

  exit(0);
}
