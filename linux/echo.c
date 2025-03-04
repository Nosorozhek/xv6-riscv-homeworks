#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static int min(int x, int y) { return x < y ? x : y; }

#define BUFFER_SIZE 16384

static void flush_buffer(int fd, char *buffer, int length) {
  int bytes_written = 0;
  while (bytes_written < length) {
    int result = write(fd, buffer + bytes_written, length);
    if (result < 0) {
      fprintf(stderr, "echo: failed to write buffer into a pipe.\n");
      exit(1);
    }
    bytes_written += result;
  }
}

static void write_all(int fd, const char *data, int length, char *buffer,
                      char **buffer_ptr) {
  while (length) {
    int remaining_space = BUFFER_SIZE - (*buffer_ptr - buffer);
    if (remaining_space == 0) {
      flush_buffer(fd, buffer, BUFFER_SIZE);
      *buffer_ptr = buffer;
      remaining_space = BUFFER_SIZE;
    }

    int copied_length = min(remaining_space, length);
    memcpy(*buffer_ptr, (void *)data, copied_length);
    length -= copied_length;
    data += copied_length;
    *buffer_ptr += copied_length;
  }
}

int main(int argc, char **argv) {
  int pipe_fd[2];

  if (pipe(pipe_fd) < 0) {
    fprintf(stderr, "echo: failed to create a pipe.\n");
    exit(1);
  }

  int pid = fork();

  if (pid < 0) {
    fprintf(stderr, "echo: failed to create a child process.\n");
    exit(1);

  } else if (pid == 0) {
    close(pipe_fd[1]);

    close(0);
    dup(pipe_fd[0]);

    char reading_buffer[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char *buffer_ptr = buffer;
    int len;
    while ((len = read(0, &reading_buffer, BUFFER_SIZE)) > 0) {
      write_all(1, reading_buffer, len, buffer, &buffer_ptr);
    }
    if (len < 0) {
      fprintf(stderr, "echo: failed to read from stdin.\n");
    }
    flush_buffer(1, buffer, buffer_ptr - buffer);

  } else {
    close(pipe_fd[0]);
    char buffer[BUFFER_SIZE];
    char *buffer_ptr = buffer;

    for (int i = 0; i < argc; ++i) {
      int length = strlen(argv[i]);
      write_all(pipe_fd[1], argv[i], length, buffer, &buffer_ptr);
      if (i < argc - 1) {
        write_all(pipe_fd[1], " ", 1, buffer, &buffer_ptr);
      } else {
        write_all(pipe_fd[1], "\n", 1, buffer, &buffer_ptr);
      }
    }
    flush_buffer(pipe_fd[1], buffer, buffer_ptr - buffer);
    if (close(pipe_fd[1])) {
      fprintf(stderr, "echo: write error.\n");
      exit(1);
    }
    int exit_status;
    wait(&exit_status);
    if (exit_status != 0) {
      fprintf(stderr, "echo: child exited with status %d", exit_status);
      exit(1);
    }
  }

  exit(0);
}
