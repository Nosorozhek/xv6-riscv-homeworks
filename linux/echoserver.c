#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sighandler.h"

static int daemon_mode = 0;
static char *output_file = "echoserver.out";
static char *log_file = "echoserver.log";
static int ping_duration = 1;

#ifndef FIFO_FILE
#define FIFO_FILE ("fifo")
#endif  // FIFO_FILE

int parse_args(int argc, char *argv[]) {
  int opt;

  static struct option long_options[] = {
    {"help", 0, 0, 0}, {"daemon", 0, 0, 0}, {0, 0, 0, 0}
  };

  int index = 0;
  while ((opt = getopt_long(argc, argv, "o:n:l:", long_options, &index)) !=
         -1) {
    switch (opt) {
      case 0:
        if (strcmp("daemon", long_options[index].name) == 0) {
          daemon_mode = 1;
        } else if (strcmp("help", long_options[index].name) == 0) {
          fprintf(stdout,
                  "Usage: %s [--daemon] [-o output file] [-n ping duration in "
                  "seconds] [-l log file]\n",
                  argv[0]);
          exit(EXIT_SUCCESS);
        }
        break;
      case 'o':
        output_file = optarg;
        break;
      case 'n':
        ping_duration = atoi(optarg);
        break;
      case 'l':
        log_file = optarg;
        break;
      default:
        fprintf(stderr,
                "Usage: %s [--daemon] [-o output file] [-n ping duration in "
                "seconds] [-l log file]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  return 0;
}

static size_t messages_count = 0;
static size_t bytes_count = 0;

void process_interrupt(int fifo_fd);

void print_statistics() {
  printf("Server statistics\nMessages received: %zu\nBytes read:        %zu\n",
         messages_count, bytes_count);
  sigalrm_received = 0;
}

#define print_safe(string)                 \
  while (printf(string) < 0) {             \
    if (errno != EINTR) {                  \
      perror("Failed to print to stdout"); \
      exit(EXIT_FAILURE);                  \
    }                                      \
    process_interrupt(fifo_fd);            \
  }

#define with_interrupt_check(operation, message) \
  while (operation) {                            \
    if (errno != EINTR) {                        \
      perror(message);                           \
      exit(EXIT_FAILURE);                        \
    }                                            \
    process_interrupt(fifo_fd);                  \
  }

int daemonize(const int fifo_fd) {
  int output_fd;
  with_interrupt_check((output_fd = creat(output_file, DEFFILEMODE)) < 0,
                 "Failed to open output file");

  // Fflush stdout and stderr buffers before dup2, because sometimes
  // they might be printed into the output file after dup2.
  // Btw man page says that dup2 calls close so it should fflush
  // stdout before dup, but it doesn't happen in my test.
  with_interrupt_check(fflush(stdout), "Failed to fflush stdout");
  with_interrupt_check(fflush(stderr), "Failed to fflush stderr");

  with_interrupt_check(dup2(output_fd, STDOUT_FILENO) == -1, "Failed to redirect stdout to output file");
  with_interrupt_check(dup2(output_fd, STDERR_FILENO) == -1, "Failed to redirect stderr to output file");

  // Do not change the working directory because this will change the path to
  // the fifo file. Do not close the file descriptors because they are inherited
  // by the daemonized process.
  with_interrupt_check(daemon(1, 1), "Failed to demonize the process");

  // Restart the alarm because it is not inherited by children created via fork,
  // so it is not expected to be inherited by the daemonized process.
  alarm(ping_duration);

  print_safe("Process is daemonized.\n");
  print_statistics();

  return 0;
}


static int have_to_exit = 0;

void close_fifo(const int fifo_fd) {
  with_interrupt_check(close(fifo_fd) == -1, "Failed to close fifo file");
}

void process_interrupt(const int fifo_fd) {
  if (sigint_received) {
    sigint_received = 0;
    if (fifo_fd == -1) {
      print_safe(
        "SIGINT received. At the moment no data is being read. "
        "Terminating the process.\n");
      print_statistics();
      exit(EXIT_FAILURE);
    }
    print_safe(
      "SIGINT received. The process will be read to the end, after "
      "which the process will be terminated.\n");
    have_to_exit = 1;
  }
  if (sigterm_received) {
    sigterm_received = 0;
    print_safe("SIGTERM received. Terminating the process.\n");
    if (fifo_fd != -1) {
      close_fifo(fifo_fd);
    }
    print_statistics();
    exit(EXIT_FAILURE);
  }
  if (sigalrm_received) {
    sigalrm_received = 0;
    alarm(ping_duration);
    print_safe("SIGALRM received. Server is working.\n");
  }
  if (sighup_received) {
    sighup_received = 0;
    if (!daemon_mode) {
      daemon_mode = 1;
      print_safe("SIGHUP received. Demonizing the process.\n");
      daemonize(fifo_fd);
    } else {
      print_safe("SIGHUP received. Process is already a daemon.\n");
    }
  }
  if (sigusr1_received) {
    sigusr1_received = 0;
    print_safe("SIGUSR1 received. Printing server statistics.\n");
    print_statistics();
  }
}

#define check_interrupt(message) \
  if (errno != EINTR) {          \
    perror(message);             \
    exit(EXIT_FAILURE);          \
  }                              \
  process_interrupt(fifo_fd);

static int log_fd;

void process_data() {
  process_interrupt(-1);

  int fifo_fd;
  while ((fifo_fd = open(FIFO_FILE, O_RDONLY)) == -1) {
    check_interrupt("Failed to open fifo file");
    if (have_to_exit) {
      return;
    }
  }

  bool is_eof = false;
  char last_char = '\0';
  while (!is_eof) {
    char buffer[1024];
    ssize_t bytes_read = read(fifo_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
      check_interrupt("Failed to read from fifo file")
      continue;
    }
    if (bytes_read == 0) {
      ++messages_count;
      is_eof = true;
    }
    bytes_count += bytes_read;

    if (!is_eof) {
      last_char = buffer[bytes_read - 1];
    } else if (last_char != '\n') {
      buffer[bytes_read++] = '\n';
    }

    char *buffer_begin = buffer;
    while (bytes_read > 0) {
      ssize_t bytes_written = write(log_fd, buffer_begin, bytes_read);
      if (bytes_written == -1) {
        check_interrupt("Failed to write to log file");
      } else {
        bytes_read -= bytes_written;
        buffer_begin += bytes_written;
      }
    }

    process_interrupt(fifo_fd);
    if (have_to_exit) {
      break;
    }
  }

  close_fifo(fifo_fd);
}

void create_fifo() {
  if (mkfifo(FIFO_FILE, 0600) == -1) {
    if (errno != EEXIST) {
      perror("Failed to create new fifo pipe");
      exit(EXIT_FAILURE);
    }

    struct stat stat_buf;
    if (stat(FIFO_FILE, &stat_buf) == -1) {
      perror("Failed to stat fifo pipe");
      exit(EXIT_FAILURE);
    }

    if (!S_ISFIFO(stat_buf.st_mode)) {
      perror("File is not a fifo pipe");
      exit(EXIT_FAILURE);
    }
  }
}

void open_log_file() {
  while ((log_fd = creat(log_file, DEFFILEMODE)) == -1) {
    if (errno != EINTR) {
      perror("Failed to open log file");
      exit(EXIT_FAILURE);
    }
    process_interrupt(-1);
    if (have_to_exit) {
      return;
    }
  }
}

void close_log_file() {
  while (close(log_fd) == -1) {
    if (errno != EINTR) {
      perror("Failed to close log file");
      exit(EXIT_FAILURE);
    }
    process_interrupt(-1);
  }
}

int main(int argc, char **argv) {
  parse_args(argc, argv);;
  alarm(ping_duration);
  register_sighandler();

  if (daemon_mode) {
    daemonize(-1);
  }

  create_fifo();

  open_log_file();
  while (!have_to_exit) {
    process_data();
  }
  close_log_file();

  printf("Terminating the process.\n");
  print_statistics();
  return 0;
}
