#include "kernel/types.h"
#include "user/user.h"

#define NULL 0

int is_digit(char c) {
  return '0' <= c && c <= '9';
}

int main(int argc, char *argv[]) {
  const int max = 512;
  char buf[max];
  
  int i;
  char c;
  char* first_ptr = NULL;
  char* second_ptr = NULL;
  for (i = 0; i + 1 < max; ++i) {
    int cc = read(0, &c, 1);
    if (cc < 0) {
      fprintf(2, "add: read error\n");
      exit(1);
    }
    if (cc == 0) {
      buf[i] = '\0';
      break;
    }
    buf[i] = c;

    if (c == '\n' || c == '\r'){
      buf[i] = '\0';
      break;
    }

    if(first_ptr == NULL && is_digit(c)){
      first_ptr = buf + i;
      continue;
    }
    if(first_ptr != NULL && second_ptr == NULL && !is_digit(buf[i - 1]) && is_digit(c))
      second_ptr = buf + i;
  }
  
  printf("|%s|\n", buf);

  int first = atoi(first_ptr);
  int second = atoi(second_ptr);
  int res = add(first, second);   // do syscall
  printf("%d\n", res);
  exit(0);
}
