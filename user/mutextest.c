#include "kernel/types.h"
#include "user/user.h"

#define assert(statement, test_name)                              \
  if (!(statement)) {                                             \
    fprintf(2, "mutextest: [%s] assertion failed.\n", test_name); \
    return -1;                                                    \
  }

#define try(statement, message, test_name)                   \
  if (statement) {                                           \
    fprintf(2, "mutextest: [%s] %s.\n", test_name, message); \
    return -1;                                               \
  }

#define try_child(statement, message, test_name)             \
  if (statement) {                                           \
    fprintf(2, "mutextest: [%s] %s.\n", test_name, message); \
    exit(1);                                                 \
  }

int wait_child() {
  int status;
  wait(&status);
  return status;
}

void run_test(int test, const char* test_name) {
  if (!(test)) {
    printf("mutextest: [%s] passed.\n", test_name);
  } else {
    printf("mutextest: [%s] failed.\n", test_name);
  }
}

int test_read_write_mutex() {
  const char* test_name = "test_read_write_mutex";
  int mtx;
  try(mutex(&mtx), "failed to create mutex", test_name);

  char buf[10];
  assert(read(mtx, buf, sizeof(buf)) == -1, test_name);
  assert(write(mtx, "test", 4) == -1, test_name);

  try(close(mtx), "failed to close mutex", test_name);

  return 0;
}

int test_close_locked_mutex_same_process() {
  const char* test_name = "test_close_locked_mutex_same_process";
  int mtx;

  try(mutex(&mtx), "failed to create mutex", test_name);
  try(mutex_lock(mtx), "failed to lock mutex", test_name);

  assert(close(mtx) == 0, test_name);

  return 0;
}

int test_close_locked_mutex_other_process() {
  const char* test_name = "test_close_locked_mutex_other_process";
  int mtx;
  try(mutex(&mtx), "failed to create mutex", test_name);

  int pid = fork();
  if (pid < 0) {
    try(0, "failed to fork the process", test_name);
  } else if (pid == 0) {
    try_child(mutex_lock(mtx), "failed to lock mutex", test_name);
    sleep(10);
    try_child(mutex_unlock(mtx), "failed to unlock mutex", test_name);
    exit(0);
  } else {
    sleep(5);
    assert(close(mtx) == 0, test_name);
    try(wait_child(), "child process exited with error", test_name);
  }

  return 0;
}

int test_process_termination_with_locked_mutex() {
  const char* test_name = "test_process_termination_with_locked_mutex";
  int mtx;
  try(mutex(&mtx), "failed to create mutex", test_name);

  int pid = fork();
  if (pid < 0) {
    try(0, "failed to fork the process", test_name);
  } else if (pid == 0) {
    try_child(mutex_lock(mtx), "failed to lock mutex", test_name);
    exit(0);
  } else {
    try(wait_child(), "child process exited with error", test_name);

    int start_time = uptime();
    try(mutex_lock(mtx), "failed to lock mutex", test_name);
    assert(uptime() - start_time < 2, test_name);

    try(mutex_unlock(mtx), "failed to unlock mutex", test_name);
    close(mtx);
  }

  return 0;
}

int test_unlock_other_process_mutex() {
  const char* test_name = "test_unlock_other_process_mutex";
  int mtx;
  try(mutex(&mtx), "failed to create mutex", test_name);

  int pid = fork();
  if (pid < 0) {
    try(0, "failed to fork the process", test_name);
  } else if (pid == 0) {
    try_child(mutex_lock(mtx), "failed to lock mutex", test_name);
    sleep(10);
    try_child(mutex_unlock(mtx), "failed to unlock mutex", test_name);
    exit(0);
  } else {
    sleep(5);
    assert(mutex_unlock(mtx) == -1, test_name);

    try(wait_child(), "child process exited with error", test_name);
    try(close(mtx), "failed to close mutex", test_name);
  }

  return 0;
}

int test_multiple_mutex_cleanup() {
  const char* test_name = "test_multiple_mutex_cleanup";
  int mtx1, mtx2, mtx3;

  try(mutex(&mtx1), "failed to create mutex 1", test_name);

  try(mutex(&mtx2), "failed to create mutex 2", test_name);

  int pid = fork();
  if (pid < 0) {
    try(0, "failed to fork the process", test_name);
  } else if (pid == 0) {
    try_child(mutex(&mtx3), "failed to create mutex 3", test_name);
    try_child(mutex_lock(mtx2), "failed to lock mutex 2", test_name);

    exit(0);  // mutex 3 must be closed and deallocated by kernel
  } else {
    try(wait_child(), "child process exited with error", test_name);

    try(close(mtx1), "failed to close mutex 1", test_name);
    try(close(mtx2), "failed to close mutex 2", test_name);
  }

  return 0;
}

int main() {
  mutex_debug(1);
  run_test(test_read_write_mutex(), "test_read_write_mutex");
  run_test(test_close_locked_mutex_same_process(),
           "test_close_locked_mutex_same_process");
  run_test(test_close_locked_mutex_other_process(),
           "test_close_locked_mutex_other_process");
  run_test(test_process_termination_with_locked_mutex(),
           "test_process_termination_with_locked_mutex");
  run_test(test_unlock_other_process_mutex(),
           "test_unlock_other_process_mutex");
  run_test(test_multiple_mutex_cleanup(), "test_multiple_mutex_cleanup");
  mutex_debug(0);
  exit(0);
}
