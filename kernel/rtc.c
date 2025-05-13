#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"

#define ReadRTC(word) (*((volatile uint32 *)(word)))

static struct spinlock lock;

void rtcinit(void) {
  initlock(&lock, "rtc");
}

uint64 rtcread(void) {
  acquire(&lock);
  // read low register first
  const uint32 low = ReadRTC(RTC_LOW);
  const uint32 high = ReadRTC(RTC_HIGH);
  release(&lock);
  return ((uint64)high << 32) | low;
}
