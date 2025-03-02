#include "kernel/types.h"
#include "user/user.h"

static int is_digit(char c) {
  return '0' <= c && c <= '9';
}

static int read_string(char *buffer, int buffer_size) {
  int i;
  char c;
  for (i = 0; i < buffer_size; ++i) {
    int cc = read(0, &c, 1);
    if (cc < 0) {
      fprintf(2, "add: failed to read input\n");
      exit(1);
    } 
    if (cc == 0 || c == '\n')
      break;

    buffer[i] = c;
  }
  if(i == buffer_size) {
    fprintf(2, "add: input buffer overflow\n");
    exit(1);
  }
  buffer[i] = '\0';
  return 0;
} 

static const char *parse_number(const char *begin, const char *s, char trailing) {
  do {
    if(!is_digit(*s)) {
      char c[] = {*s, '\0'};
      fprintf(2, "add: Parsing error on character %d \'%s\'. Incorrect input format.\n\
There must be two numbers on a single line, separated by a single \
whitespace, with no other characters such as trailing whitespaces.\n", 
        (int)(s - begin + 1), c);
      exit(1);
    }
  } while (*(++s) != trailing);
  return s;
}

int main() {
  const int BUFFER_SIZE = 32; 
  char buf[BUFFER_SIZE + 1];
  read_string(buf, BUFFER_SIZE);
  printf("|%s|\n", buf);
  
  const char *ptr = buf;
  int first_sign = 1;
  if (*ptr == '-') {
      first_sign = -1;
      ptr++;
  }
  const char *first_ptr = ptr;

  
  ptr = parse_number(buf, ptr, ' ');
  
  ++ptr;
  
  int second_sign = 1;
  if (*ptr == '-') {
      second_sign = -1;
      ++ptr;
  }
  const char *second_ptr = ptr;

  ptr = parse_number(buf, ptr, '\0');

  int first = atoi(first_ptr) * first_sign;
  int second = atoi(second_ptr) * second_sign;
  int sum = add(first, second);                   // do syscall
  printf("%d\n", sum);
  exit(0);
}
