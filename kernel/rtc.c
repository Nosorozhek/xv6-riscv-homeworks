#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"

#define RTCWord(word)
#define ReadRTCLow (*((volatile uint32 *)(RTC_LOW)))
#define ReadRTCHigh (*((volatile uint32 *)(RTC_HIGH)))

uint64 rtcread(void) {
  return ((uint64)ReadRTCHigh << 32) + ReadRTCLow;
}
