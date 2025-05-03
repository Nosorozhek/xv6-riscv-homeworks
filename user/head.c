#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUF_SIZE 512
char buf[BUF_SIZE];

int head(const int fd, int byte_limit) {
  int read_result;
  int to_read = ((byte_limit == -1) || (byte_limit >= BUF_SIZE)) ? BUF_SIZE : byte_limit;
  while (to_read > 0 && (read_result = read(fd, buf, to_read)) != 0) {
    if (read_result < 0) {
      fprintf(2, "head: read error\n");
      exit(1);
    }
    if (write(1, buf, read_result) != read_result) {
      fprintf(2, "head: write error\n");
      exit(1);
    }
    if (byte_limit != -1) {
      byte_limit -= read_result;
    }
    to_read = byte_limit == -1 || byte_limit >= BUF_SIZE ? BUF_SIZE : byte_limit;
  }
  if (write(1, "\n", 1) != 1) {
    fprintf(2, "head: write error\n");
    exit(1);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int byte_limit = -1;
  if (argc < 2) {
    fprintf(2, "usage: head [-n number_of_bytes] [files]\n");
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0) {
      if (i + 1 >= argc) {
        fprintf(2, "head: -n requires an argument\n");
        exit(1);
      }

      byte_limit = atoi(argv[i + 1]);
      if (byte_limit < 0) {
        fprintf(2, "head: -n argument must be non-negative number: %s\n", argv[i + 1]);
        exit(1);
      }
      ++i;
    }
  }

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-n") == 0) {
      ++i;
      continue;
    }

    int fd;
    if ((fd = open(argv[i], O_RDONLY)) < 0) {
      fprintf(2, "head: cannot open %s\n", argv[i]);
      exit(1);
    }

    head(fd, byte_limit);
    if (close(fd)) {
      fprintf(2, "head: close error\n");
      exit(1);
    }
  }

  exit(0);
}
