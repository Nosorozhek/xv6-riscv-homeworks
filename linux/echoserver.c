#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sighandler.h"

static int daemon_mode = 0;
static char *output_file = NULL;

#ifndef FIFO_FILE
#define FIFO_FILE ("fifo")
#endif // FIFO_FILE

int parse_args(int argc, char *argv[]) {
  int opt;

  static struct option long_options[] = {
      {"help", 0, 0, 0}, {"daemon", 0, 0, 0}, {0, 0, 0, 0}};

  int index = 0;
  while ((opt = getopt_long(argc, argv, "o:n:", long_options, &index)) != -1) {
    switch (opt) {
      case 0:
        if (strcmp("daemon", long_options[index].name) == 0) {
          daemon_mode = 1;
        } else if (strcmp("help", long_options[index].name) == 0) {
          fprintf(
              stdout,
              "Usage: %s [--daemon] [-o output_file] [-n number_of_seconds]\n",
              argv[0]);
          exit(EXIT_SUCCESS);
        }
        break;
      case 'o':
        output_file = optarg;
        break;
      case 'n':
        number_of_seconds = atoi(optarg);
        break;
      default:
        fprintf(
            stderr,
            "Usage: %s [--daemon] [-o output_file] [-n number_of_seconds]\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  return 0;
}

int daemonize() {
  int output_fd = creat(output_file, DEFFILEMODE);
  if (output_fd < 0) {
    fprintf(stderr, "Failed to open %s: %s", output_file, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (dup2(output_fd, STDOUT_FILENO) == -1) {
    perror("Failed to redirect stdout to output file");
    exit(EXIT_FAILURE);
  }
  if (dup2(output_fd, STDERR_FILENO) == -1) {
    perror("Failed to redirect stdout to output file");
    exit(EXIT_FAILURE);
  }

  // Do not change the working directory because this will change the path to
  // the fifo file. Do not close the file descriptors because they are inherited
  // by the daemonized process.
  if (daemon(1, 1)) {
    perror("Failed to demonize the process");
    exit(EXIT_FAILURE);
  }

  // Restart the alarm because it is not inherited by children created via fork,
  // so it is not expected to be inherited by the daemonized process.
  alarm(number_of_seconds);

  return 0;
}

static size_t messages_count = 0;
static size_t bytes_count = 0;

void print_statistics() {
  printf("Server statistics\nMessages received: %zu\nBytes read:        %zu\n",
         messages_count, bytes_count);
  sigalrm_received = 0;
}

int have_to_exit = 0;

void process_interrupt(int fifo_fd) {
  if (sigint_received) {
    sigint_received = 0;
    if (fifo_fd == -1) {
      printf(
          "SIGINT received. At the moment no data is being read. "
          "Terminating the process.\n");
      print_statistics();
      exit(EXIT_FAILURE);
    }
    printf(
        "SIGINT received. The process will be read to the end, after "
        "which the process will be terminated.\n");
    have_to_exit = 1;
  }
  if (sigterm_received) {
    sigterm_received = 0;
    printf("SIGTERM received. Terminating the process.\n");
    print_statistics();
    exit(EXIT_FAILURE);
  }
  if (sigalrm_received) {
    sigalrm_received = 0;
    alarm(number_of_seconds);
    printf("SIGALRM received. Server is working.\n");
  }
  if (sighup_received) {
    sighup_received = 0;
    if (!daemon_mode) {
      printf("SIGHUP received. Daemonizing the process.\n");
      daemon_mode = 1;
      daemonize();
    } else {
      printf("SIGHUP received. Process is already a daemon.\n");
    }
  }
  if (sigusr1_received) {
    sigusr1_received = 0;
    printf("SIGUSR1 received. Printing server statistics.\n");
    print_statistics();
  }
}

void process_data() {
  int fifo_fd;
  while ((fifo_fd = open(FIFO_FILE, O_RDONLY)) == -1) {
    if (errno != EINTR) {
      perror("Failed to open fifo file");
      exit(EXIT_FAILURE);
    }
    process_interrupt(fifo_fd);
  }

  char buffer[1025];
  ssize_t bytes_read;

  do {
    bytes_read = read(fifo_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
      if (errno != EINTR) {
        perror("Failed to read from fifo file");
        exit(EXIT_FAILURE);
      }
      process_interrupt(fifo_fd);
    } else if (bytes_read > 0) {
      bytes_count += bytes_read;
      printf(">> %s", buffer);
    }
  } while (bytes_read > 0);

  int close_result;
  while ((close_result = close(fifo_fd)) == -1) {
    if (errno != EINTR) {
      perror("Failed to close fifo file");
      exit(EXIT_FAILURE);
    }
    process_interrupt(fifo_fd);
  }
  ++messages_count;
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

int main(int argc, char **argv) {
  parse_args(argc, argv);
  register_sighandler();
  create_fifo();
  if (daemon_mode) {
    daemonize();
  }

  alarm(number_of_seconds);

  while (!have_to_exit) {
    process_data();
  }
  printf("Terminating the process.\n");
  print_statistics();
  return 0;
}