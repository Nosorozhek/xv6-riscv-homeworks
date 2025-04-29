#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"

#define ReadRTC(word) (*((volatile uint32 *)(word)))

uint64 rtcread(void) {
  // read low register first
  const uint32 low = ReadRTC(RTC_LOW);
  const uint32 high = ReadRTC(RTC_HIGH);

  return ((uint64)high << 32) | low;
}
