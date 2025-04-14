#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

static const int MAX_LEVEL = 2;

char *compute_flags(char *flags, pte_t *pte) {
  flags[0] = (*pte & PTE_R) ? 'R' : '_';
  flags[1] = (*pte & PTE_W) ? 'W' : '_';
  flags[2] = (*pte & PTE_X) ? 'X' : '_';
  flags[3] = (*pte & PTE_U) ? 'U' : '_';
  flags[4] = (*pte & PTE_G) ? 'G' : '_';
  flags[5] = (*pte & PTE_A) ? 'A' : '_';
  flags[6] = (*pte & PTE_D) ? 'D' : '_';
  flags[7] = '\0';
  return flags;
}

char *format_index(char *buffer, uint64 index) {
  buffer[0] = '0';
  buffer[1] = 'x';

  if ((buffer[4] = index % 16 + '0') > '9') {
    buffer[4] += 'a' - '9' - 1;
  };
  index /= 16;
  if ((buffer[3] = index % 16 + '0') > '9') {
    buffer[3] += 'a' - '9' - 1;
  };
  index /= 16;
  if ((buffer[2] = index % 16 + '0') > '9') {
    buffer[2] += 'a' - '9' - 1;
  };
  return buffer;
}

void print_reqursive(pagetable_t pagetable, uint64 va, uint64 last_va,
                     int level, int flags_mask) {
  last_va = PGROUNDUPLVL(level, last_va);
  pte_t *pte = pagetable + PX(level, va);
  for (; va < last_va && pte < pagetable + PTECOUNT; va += PAGESIZE(level), ++pte) {
    
    if (!(*pte & PTE_V)) {
      continue;
    }

    if (level == 0 && flags_mask != 0 && (*pte & (uint64)flags_mask) == 0) {
      continue;
    }

    for (int i = level; i < MAX_LEVEL; ++i) { printf("      "); }
    printf("%s -> %p %s\n", format_index("0x000", PX(level, va)),
           (void *)PTE2PA(*pte), compute_flags("rwxugad", pte));

    if (level > 0) {
      print_reqursive((pagetable_t)PTE2PA(*pte), va, last_va, level - 1,
                      flags_mask);
    }
  }
}

uint64 sys_pagedump(void) {
  uint64 buf;
  argaddr(0, &buf);
  uint64 len;
  argaddr(1, &len);

  if (buf == 0 || len == 0) {
    buf = 0;
    len = MAXVA;
  }

  if (buf + len > MAXVA) {
    return -1;
  }

  int flags_mask;
  argint(2, &flags_mask);

  if (flags_mask == 1) {
    flags_mask = PTE_D;
  } else if (flags_mask == 2) {
    flags_mask = PTE_A;
  } else if (flags_mask == 3) {
    flags_mask = PTE_D | PTE_A;
  } else if (flags_mask != 0) {
    return -1;
  }

  struct proc *p = myproc();
  pagetable_t pagetable = p->pagetable;
  printf("PAGETABLE %p\n", pagetable);

  print_reqursive(pagetable, buf, buf + len, MAX_LEVEL, flags_mask);

  return 0;
}

void reset_reqursive(pagetable_t pagetable, uint64 va, uint64 last_va,
                     int level, int flags_mask) {
  last_va = PGROUNDUPLVL(level, last_va);
  pte_t *pte = pagetable + PX(level, va);
  for (; va < last_va && pte < pagetable + PTECOUNT; va += PAGESIZE(level), ++pte) {
    
    if (!(*pte & PTE_V)) {
      continue;
    }

    if (level == 0) {
      *pte &= ~((uint64)flags_mask);
    }

    if (level > 0) {
      reset_reqursive((pagetable_t)PTE2PA(*pte), va, last_va, level - 1,
                      flags_mask);
    }
  }
}

uint64 sys_pagereset(void) {
  uint64 buf;
  argaddr(0, &buf);
  uint64 len;
  argaddr(1, &len);

  if (buf == 0 || len == 0) {
    buf = 0;
    len = MAXVA;
  }

  if (buf + len > MAXVA) {
    return -1;
  }

  int flags_mask;
  argint(2, &flags_mask);

  if (flags_mask == DIRTY_PAGE) {
    flags_mask = PTE_D;
  } else if (flags_mask == ACCESSED_PAGE) {
    flags_mask = PTE_A;
  } else if (flags_mask == (DIRTY_PAGE | ACCESSED_PAGE)) {
    flags_mask = PTE_D | PTE_A;
  } else {
    return -1;
  }

  struct proc *p = myproc();
  pagetable_t pagetable = p->pagetable;

  reset_reqursive(pagetable, buf, buf + len, MAX_LEVEL, flags_mask);

  return 0;
}