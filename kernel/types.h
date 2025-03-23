typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long uint64;

typedef uint64 pde_t;

enum procstate_u { SLEEPING_U, RUNNABLE_U, RUNNING_U, ZOMBIE_U };

struct procinfo {
    int pid;                    // Process ID
    char name[16];              // Process name
    enum procstate_u state;     // Process state
    int ppid;                   // Parent process ID
    char pname[16];             // Parent process name
};