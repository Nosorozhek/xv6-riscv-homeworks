#ifndef SIGHANDLER_H
#define SIGHANDLER_H

#include <signal.h>

volatile sig_atomic_t sigint_received = 0;
volatile sig_atomic_t sigterm_received = 0;
volatile sig_atomic_t sigalrm_received = 0;
volatile sig_atomic_t sighup_received = 0;
volatile sig_atomic_t sigusr1_received = 0;

void signal_handler(int signum) {
  if (signum == SIGINT) {
    sigint_received = 1;
  } else if (signum == SIGTERM) {
    sigterm_received = 1;
  } else if (signum == SIGALRM) {
    sigalrm_received = 1;
  } else if (signum == SIGHUP) {
    sighup_received = 1;
  } else if (signum == SIGQUIT) {
    // Ignore
  } else if (signum == SIGUSR1) {
    sigusr1_received = 1;
  }

}

void register_sighandler() {
  struct sigaction sa = {.sa_handler = signal_handler, .sa_flags = 0};
  sigemptyset(&sa.sa_mask);

  sigaction(SIGINT, &sa, 0);
  sigaction(SIGTERM, &sa, 0);
  sigaction(SIGALRM, &sa, 0);
  sigaction(SIGHUP, &sa, 0);
  sigaction(SIGQUIT, &sa, 0);
  sigaction(SIGUSR1, &sa, 0);
}

#endif  // SIGHANDLER_H