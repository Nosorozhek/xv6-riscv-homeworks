#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]) {
  long len = strlen(argv[1]);
  if (write(1, argv[1], (int) len) != len) {
    fprintf(2, "write: write error\n");
    exit(1);
  }
  exit(0);
}
