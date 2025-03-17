#include "kernel/types.h"
#include "user/user.h"

int main() {
  const int buffer_size = 64;
  struct procinfo *buffer = malloc(buffer_size * sizeof(struct procinfo));
  if (buffer == 0) {
    fprintf(2, "ps: failed to allocate enough memory for the buffer.\n");
    exit(1);
  }
  int res = ps_listinfo((char *)buffer, buffer_size);
  if (res < 0) {
    fprintf(2, "ps: failed to get the list of processes.\n");
    exit(1);
  }

  printf("PID\tNAME\tSTATE\t\tPPID\tPNAME\n");
  for (struct procinfo *p = buffer; p < buffer + res; ++p) {
    char *pstate = "\t\t";
    switch (p->state) {
      case SLEEPING_U:
        pstate = "SLEEPING";
        break;
      case RUNNABLE_U:
        pstate = "RUNNABLE";
        break;
      case RUNNING_U:
        pstate = "RUNNING\t";
        break;
      case ZOMBIE_U:
        pstate = "ZOMBIE\t";
        break;
    }
    printf("%d\t%s\t%s\t%d\t%s\n", p->pid, p->name, pstate, p->ppid,
           p->pname);
  }
  exit(0);
}