#include <signal.h>

#include "sighandler.h"

volatile sig_atomic_t sigint_received;
volatile sig_atomic_t sigterm_received;
volatile sig_atomic_t sigalrm_received;
volatile sig_atomic_t sighup_received;
volatile sig_atomic_t sigusr1_received;

void signal_handler(int signum) {
  if (signum == SIGINT) {
    sigint_received = 1;
  } else if (signum == SIGTERM) {
    sigterm_received = 1;
  } else if (signum == SIGALRM) {
    sigalrm_received = 1;
  } else if (signum == SIGHUP) {
    sighup_received = 1;
  } else if (signum == SIGUSR1) {
    sigusr1_received = 1;
  }
}

void register_sighandler() {
  sigint_received = 0;
  sigterm_received = 0;
  sigalrm_received = 0;
  sighup_received = 0;
  sigusr1_received = 0;

  struct sigaction sa = {.sa_handler = signal_handler, .sa_flags = 0};
  sigemptyset(&sa.sa_mask);

  sigaction(SIGINT, &sa, 0);
  sigaction(SIGTERM, &sa, 0);
  sigaction(SIGALRM, &sa, 0);
  sigaction(SIGHUP, &sa, 0);
  sigaction(SIGUSR1, &sa, 0);

  sa.sa_handler = SIG_IGN;
  sigaction(SIGQUIT, &sa, 0);
}
