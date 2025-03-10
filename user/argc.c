#include "kernel/types.h"
#include "user/user.h"

#define BUFFER_SIZE 3

int main(const int argc, const char **argv) {
  int pipe_fd[2];

  if (pipe(pipe_fd) < 0) {
    fprintf(2, "argc: failed to create a pipe.\n");
    exit(1);
  }

  int pid = fork();

  if (pid < 0) {
    fprintf(2, "argc: failed to create a child process.\n");
    exit(1);

  } else if (pid == 0) {
    if (close(pipe_fd[1])) {
      fprintf(
          2,
          "argc: failed to close the pipe write end in the child process.\n");
      exit(1);
    }

    if (close(0)) {
      fprintf(2,
              "argc: failed to close the standard input channel in the child "
              "process.\n");
      exit(1);
    }
    if (dup(pipe_fd[0]) < 0) {
      fprintf(2,
              "argc: failed to substitute the standard input channel with pipe "
              "in the child process.\n");
      exit(1);
    }
    if (close(pipe_fd[0])) {
      fprintf(
          2,
          "argc: failed to close the pipe read end in the child process.\n");
      exit(1);
    }

    char *argv[] = {"/wc", 0};
    exec("/wc", argv);

    fprintf(2, "argc: failed to execute /wc.\n");
    exit(1);
  } else {
    if (close(pipe_fd[0])) {
      fprintf(
          2,
          "argc: failed to close the pipe read end.\n");
      exit(1);
    }

    for (int i = 0; i < argc; ++i) {
      int to_write = strlen(argv[i]);
      int written = 0;
      while (to_write > 0) {
        written += write(pipe_fd[1], argv[i] + written, to_write);
        if (written < 0) {
          fprintf(2, "argc: failed to write into the pipe.\n");
          exit(1);
        }
        to_write -= written;
      }
      if (i < argc - 1) {
        written = write(pipe_fd[1], " ", 1);
      } else {
        written = write(pipe_fd[1], "\n", 1);
      }
      if (written < 0) {
        fprintf(2, "argc: failed to write into the pipe.\n");
        exit(1);
      }
    }
    if (close(pipe_fd[1])) {
      fprintf(2, "argc: failed to close the pipe.\n");
      exit(1);
    }
    int exit_status;
    wait(&exit_status);
    if (exit_status != 0) {
      fprintf(2, "argc: child exited with status %d.", exit_status);
      exit(1);
    }
  }

  exit(0);
}
