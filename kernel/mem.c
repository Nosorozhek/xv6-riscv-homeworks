#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

static int
nullwrite(int user_src, uint64 src, int n, short minor) {
  if (n < 0) {
    return -1;
  }
  return n;
}

static int
nullread(int user_dst, uint64 dst, int n, short minor) {
  if (n < 0) {
    return -1;
  }
  return 0;
}


static int
zerowrite(int user_src, uint64 src, int n, short minor) {
  return -1;
}

#define BUF_SIZE 1024

static int
zeroread(int user_dst, uint64 dst, int n, short minor) {
  if (n < 0) {
    return -1;
  }
  char buf[BUF_SIZE];
  for (char *p = buf; p != buf + BUF_SIZE; ++p) {
    *p = 0;
  }
  if (n < 0) {
    return -1;
  }

  uint target = n;
  while (n > 0) {
    int len = BUF_SIZE;
    if (len > n) {
      len = n;
    }

    if (either_copyout(user_dst, dst, &buf, len) == -1) {
      return -1;
    }
    n -= len;
  }
  return target - n;
}

static const uint64 a = 6364136223846793005;
static const uint64 c = 1442695040888963407;
static uint64 seed = 1234;
static char urandom_buf[BUF_SIZE];
static char *urandom_ptr = urandom_buf;
static struct sleeplock urandom_lock;

static void regenerate() {
  for (uint64 *p = (uint64 *) urandom_buf; p != (uint64 *) (urandom_buf + BUF_SIZE); ++p) {
    seed = a * seed + c;
    *p = seed;
  }
  urandom_ptr = urandom_buf;
}

static int
urandomwrite(int user_src, uint64 src, int n, short minor) {
  if (n != sizeof(uint64)) {
    return -1;
  }
  uint64 new_seed;
  if (either_copyin(&new_seed, user_src, src, sizeof(uint64)) == -1) {
    return 0;
  }

  acquiresleep(&urandom_lock);
  seed = new_seed;
  regenerate();
  releasesleep(&urandom_lock);

  return n;
}

static int
urandomread(int user_dst, uint64 dst, int n, short minor) {
  if (n < 0) {
    return -1;
  }

  uint target = n;

  acquiresleep(&urandom_lock);
  while (n > 0) {
    if (urandom_ptr == urandom_buf + BUF_SIZE) {
      regenerate();
    }
    uint len = urandom_buf + BUF_SIZE - urandom_ptr;
    if (len > n) {
      len = n;
    }

    if (either_copyout(user_dst, dst, urandom_ptr, len) == -1) {
      releasesleep(&urandom_lock);
      return -1;
    }
    urandom_ptr += len;
    n -= len;
  }
  releasesleep(&urandom_lock);

  return target - n;
}

static uint64 bytes_read = 0;
static struct sleeplock nullstat_lock;

static int
nullstatwrite(int user_src, uint64 src, int n, short minor) {
  if (n < 0) {
    return -1;
  }

  acquiresleep(&nullstat_lock);
  bytes_read += n;
  releasesleep(&nullstat_lock);

  return n;
}

static int
nullstatread(int user_dst, uint64 dst, int n, short minor) {
  if (n != sizeof(uint64)) {
    return -1;
  }

  acquiresleep(&nullstat_lock);
  int result = n;
  if (either_copyout(user_dst, dst, &bytes_read, sizeof(uint64)) == -1) {
    result = 0;
  }
  releasesleep(&nullstat_lock);

  return result;
}

static struct devsw devices[] = {
  [DEV_NULL]{nullread, nullwrite},
  [DEV_ZERO]{zeroread, zerowrite},
  [DEV_URANDOM]{urandomread, urandomwrite},
  [DEV_NULLSTAT]{nullstatread, nullstatwrite},
};

static int
memwrite(int user_src, uint64 src, int n, short minor) {
  return devices[minor].write(user_src, src, n, minor);
}

static int
memread(int user_dst, uint64 dst, int n, short minor) {
  return devices[minor].read(user_dst, dst, n, minor);
}

void
meminit(void) {
  initlock(&urandom_lock.lk, "urandom");
  initlock(&nullstat_lock.lk, "nullstat");
  regenerate();

  devsw[MEM].read = memread;
  devsw[MEM].write = memwrite;
}
