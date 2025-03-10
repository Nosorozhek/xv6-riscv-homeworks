#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 16384

int main(int argc, char **argv) {
  int pipe_fd[2];

  if (pipe(pipe_fd) < 0) {
    perror("echo: failed to create a pipe.\n");
    exit(1);
  }

  __pid_t pid = fork();

  if (pid < 0) {
    perror("echo: failed to create a child process.\n");
    exit(1);
  } else if (pid == 0) {
    if (close(pipe_fd[1])) {
      perror(
          "argc: failed to close the pipe write end in the child process.\n");
      exit(1);
    }

    char buffer[BUFFER_SIZE];
    ssize_t len;
    while ((len = read(pipe_fd[0], &buffer, BUFFER_SIZE)) > 0) {
      ssize_t written = 0;
      while (len > 0) {
        written += write(1, buffer + written, len);
        if (written < 0) {
          perror("argc: failed to write into the standard output.\n");
          exit(1);
        }
        len -= written;
      }
    }
    if (len < 0) {
      perror("echo: failed to read from standard input.\n");
      exit(1);
    }
  } else {
    if(close(pipe_fd[0])) {
      perror("echo: failed to close the reading end of the pipe.\n");
      exit(1);
    }

    for (int i = 0; i < argc; ++i) {
      size_t to_write = strlen(argv[i]);
      ssize_t written = 0;
      while (to_write > 0) {
        ssize_t result = write(pipe_fd[1], argv[i] + written, to_write);
        if (result < 0) {
          perror("echo: failed to write into the pipe.\n");
          exit(1);
        }
        written += result;
        to_write -= result;
      }
      if (i < argc - 1) {
        written = write(pipe_fd[1], " ", 1);
      } else {
        written = write(pipe_fd[1], "\n", 1);
      }
      if (written < 0) {
        perror("echo: failed to write into the pipe.\n");
        exit(1);
      }
    }
    if (close(pipe_fd[1])) {
      perror("echo: write error.\n");
      exit(1);
    }
    int exit_status;
    wait(&exit_status);
    if (WEXITSTATUS(exit_status) != 0) {
      fprintf(stderr, "echo: child exited with status %d.\n",
              WEXITSTATUS(exit_status));
      exit(1);
    }
  }

  exit(0);
}
