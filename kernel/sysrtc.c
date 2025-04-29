#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

uint64 sys_time(void) {
  uint64 p;
  argaddr(0, &p);
  uint64 timestamp = rtcread();

  return copyout(myproc()->pagetable, p, (char *) &timestamp, sizeof(timestamp));
}
