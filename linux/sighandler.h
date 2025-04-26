#pragma once

#include <signal.h>

extern volatile sig_atomic_t sigint_received;
extern volatile sig_atomic_t sigterm_received;
extern volatile sig_atomic_t sigalrm_received;
extern volatile sig_atomic_t sighup_received;
extern volatile sig_atomic_t sigusr1_received;

void signal_handler(int signum);

void register_sighandler();
