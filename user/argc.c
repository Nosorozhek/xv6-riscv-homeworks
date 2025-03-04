#include "kernel/types.h"
#include "user/user.h"

static int min(int x, int y) { return x < y ? x : y; }

#define BUFFER_SIZE 512

static void flush_buffer(int fd, char *buffer, int length) {
  int bytes_written = 0;
  while (bytes_written < length) {
    int result = write(fd, buffer + bytes_written, length);
    if (result < 0) {
      fprintf(2, "argc: failed to write buffer into a pipe.\n");
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
    fprintf(2, "argc: failed to create a pipe.\n");
    exit(1);
  }

  int pid = fork();

  if (pid < 0) {
    fprintf(2, "argc: failed to create a child process.\n");
    exit(1);

  } else if (pid == 0) {
    close(pipe_fd[1]);

    close(0);
    dup(pipe_fd[0]);
    close(pipe_fd[0]);

    char *argv[] = {"/wc", 0};
    exec("/wc", argv);

    fprintf(2, "argc: failed to execute /wc.\n");
    exit(1);
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
      fprintf(2, "argc: write error.\n");
      exit(1);
    }
    int exit_status;
    wait(&exit_status);
    if (exit_status != 0) {
      fprintf(2, "argc: child exited with status %d", exit_status);
      exit(1);
    }
  }

  exit(0);
}
