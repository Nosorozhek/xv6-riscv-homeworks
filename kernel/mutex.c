#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

static int MUTEX_DEBUG = 0;

static void debug(char* message, struct sleeplock *mutex) {
  if (MUTEX_DEBUG) {
    printf("mutex_debug(%p): %s.\n",mutex, message);
  }
}

void setmutexdebug(int n) { MUTEX_DEBUG = n; }

int mutexalloc(struct file **f) {
  struct sleeplock *mutex = 0;
  *f = 0;
  if ((*f = filealloc()) == 0) goto bad;
  if ((mutex = (struct sleeplock *)kalloc()) == 0) goto bad;
  debug("allocated sleeplock for mutex", mutex);
  initsleeplock(mutex, "mutex");
  (*f)->type = FD_MUTEX;
  (*f)->readable = 0;
  (*f)->writable = 0;
  (*f)->mutex = mutex;
  return 0;

bad:
  debug("mutexalloc failed", mutex);
  if (mutex) kfree((char *)mutex);
  if (*f) fileclose(*f);
  return -1;
}

void mutexclose(struct sleeplock *mutex) {
  if (holdingsleep(mutex)) {
    panic("mutexclose");
  }
  kfree((char *)mutex);
  debug("deallocated sleeplock for mutex", mutex);
}
