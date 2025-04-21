#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUF_SIZE 16

static char digits[] = "0123456789abcdef";

void print_hex_addr(uint addr) {
  char line[8];
  for (int i = 6; i >= 0; --i) {
    line[i] = digits[addr & 0xf];
    addr >>= 4;
  }
  line[7] = '\0';
  printf("%s ", line);
}

void print_word(uint16 x) {
  char word[] = {digits[x >> 12 & 0xf], digits[x >> 8 & 0xf], digits[x >> 4 & 0xf], digits[x & 0xf], 0};
  printf("%s ", word);
}

int main(int argc, char *argv[]) {
  if (argc > 2) {
    fprintf(2, "usage: hexdump [file]\n");
    exit(1);
  }

  int fd = 0;
  if (argc == 2) {
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
      fprintf(2, "hexdump: cannot open %s\n", argv[1]);
      exit(1);
    }
  }

  uchar buf[BUF_SIZE + 4];
  uint addr = 0;
  int n, i;

  while ((n = read(fd, buf, BUF_SIZE)) > 0) {
    print_hex_addr(addr);

    for (i = 0; i + 2 <= n; i += 2) {
      uint16 word = *(uint16 *) (buf + i);
      print_word(word);
    }

    if (n % 2) {
      uint16 word = buf[n - 1] & 0xff;
      print_word(word);
    }
    printf("\n");
    addr += n;
  }
  print_hex_addr(addr);
  printf("\n");

  close(fd);
  exit(0);
}
