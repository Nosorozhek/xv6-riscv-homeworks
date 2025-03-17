#include "kernel/types.h"
#include "user/user.h"

void assert(int statement, const char *test_name) {
  if (!statement) {
    fprintf(2, "pslistinfotest: [%s] assertion failed.", test_name);
    exit(1);
  }
}

void test_buffer_size() {
  const int buffer_size = 2;
  struct procinfo *buffer = malloc(buffer_size * sizeof(struct procinfo));
  if (buffer == 0) {
    fprintf(2,
            "pslistinfotest: [test_buffer_size] failed to allocate enough "
            "memory for the buffer.\n");
    return;
  }

  int pid = fork();

  if (pid < 0) {
    fprintf(2,
            "pslistinfotest: [test_buffer_size] failed to create a child "
            "process during test.\n");
    return;
  } else if (pid == 0) {
    sleep(10);
    exit(1);
  } else {
    int result = ps_listinfo((char *)buffer, buffer_size);
    assert(result > 2, "test_buffer_size");
    printf("pslistinfotest: [test_buffer_size] passed.\n");
  }
}

void test_incorrect_address() {
  int res = ps_listinfo((char *)-1, 128);
  assert(res == -1, "test_incorrect_address");
  printf("pslistinfotest: [test_incorrect_address] passed.\n");
}

void test_correctness() {
  const int buffer_size = 128;
  struct procinfo *buffer = malloc(buffer_size * sizeof(struct procinfo));
  if (buffer == 0) {
    fprintf(2,
            "pslistinfotest: [test_correctness] failed to allocate enough "
            "memory for the buffer.\n");
    return;
  }
  int res = ps_listinfo((char *)buffer, buffer_size);
  assert(res > 0 && res <= 64, "test_correctness");
  printf("pslistinfotest: [test_correctness] passed.\n");
}

void test_count_processes() {
  int res = ps_listinfo((char *)0, 128);
  assert(res > 0 && res <= 64, "test_count_processes");
  printf("pslistinfotest: [test_count_processes] passed.\n");
}

int main() {
  test_buffer_size();
  test_incorrect_address();
  test_correctness();
  test_count_processes();
  exit(0);
}