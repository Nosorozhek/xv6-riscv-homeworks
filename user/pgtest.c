#include "kernel/types.h"
#include "user/user.h"

void print_separator(const char *message) {
  printf("\n\n----- %s -----\n", message);
}

void test_pagedump_flags() {
  print_separator("Testing pagedump flags");
  printf("\nDump all pages:\n");
  pagedump(0, 0, 0);

  printf("\nDump dirty pages:\n");
  pagedump(0, 0, DIRTY_PAGE);

  printf("\nDump accessed pages:\n");
  pagedump(0, 0, ACCESSED_PAGE);

  printf("\nDump dirty and accessed pages:\n");
  pagedump(0, 0, DIRTY_PAGE | ACCESSED_PAGE);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);  // clear A/D bits after test
}

int global_var;
void test_global_var() {
  print_separator("Testing Global Variable");
  printf("\nInitial:\n");
  pagedump(0, 0, 0);

  global_var = 100;
  printf("\nAfter global variable access (write):\n");
  pagedump((const char *)&global_var, 1, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);
  printf("\nAfter resetting A/D bits:\n");
  pagedump(0, 0, 0);

  int x = global_var;
  printf("\nAfter global variable access (read):\n");
  pagedump((const char *)&global_var, 1, 0);

  global_var = x;      // on purpose to suppres unused variable warning
  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);  // clear A/D bits after test
}

void test_stack_var() {
  print_separator("Testing Stack Variable");
  printf("\nInitial:\n");
  pagedump(0, 0, 0);

  int stack_var;
  printf("\nAfter allocating:\n");
  pagedump(0, 0, 0);

  stack_var = 100;
  printf("\nAfter stack variable access (write):\n");
  pagedump((const char *)&stack_var, 1, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);
  printf("\nAfter resetting A/D bits:\n");
  pagedump(0, 0, 0);

  global_var = stack_var;
  printf("\nAfter stack variable access (read):\n");
  pagedump((const char *)&stack_var, 1, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);  // clear A/D bits after test
}

void test_stack_array() {
  print_separator("Testing Stack Array");
  printf("\nInitial:\n");
  pagedump(0, 0, 0);

  char stack_array[2000];
  printf("\nAfter allocation:\n");
  pagedump(0, 0, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);
  printf("\nAfter resetting A/D bits:\n");
  pagedump(0, 0, 0);

  stack_array[1] = 10;
  printf("\nAfter stack array element access (write):\n");
  pagedump(stack_array, sizeof(stack_array), 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);
  printf("\nAfter resetting A/D bits:\n");
  pagedump(0, 0, 0);

  global_var = stack_array[1999];
  printf("\nAfter stack array element access (read):\n");
  pagedump(stack_array, sizeof(stack_array), 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);  // clear A/D bits after test
}

void test_heap_array() {
  print_separator("Testing Heap Array");
  printf("\nInitial:\n");
  pagedump(0, 0, 0);
  uint64 heap_array_size = 4096 * 2.5;
  char *heap_array = malloc(heap_array_size);
  printf("\nAfter allocation:\n");
  pagedump(0, 0, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);
  printf("\nAfter resetting A/D bits:\n");
  pagedump(0, 0, 0);

  heap_array[4096 * 2] = 10;
  printf("\nAfter heap array element access (write):\n");
  pagedump(heap_array, heap_array_size, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);
  printf("\nAfter resetting A/D bits:\n");
  pagedump(0, 0, 0);

  global_var = heap_array[4096];
  printf("\nAfter heap array element access (read):\n");
  pagedump(heap_array, heap_array_size, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);
  printf("\nAfter resetting A/D bits:\n");
  pagedump(0, 0, 0);

  free(heap_array);
  printf("\nAfter deallocation:\n");
  pagedump(0, 0, 0);

  pagereset(0, 0, DIRTY_PAGE | ACCESSED_PAGE);  // clear A/D bits after test
}

int main() {
  test_pagedump_flags();
  test_global_var();
  test_stack_var();
  test_stack_array();
  test_heap_array();
  exit(0);
}