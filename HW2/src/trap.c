#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;


/**
 * @brief Each trap string name.
 */
static const char *trapnames[] = {
    "Divide error                 ",
    "Debug exception              ",
    "Non-maskable interrupt       ",
    "Breakpoint                   ",
    "Overflow                     ",
    "Bounds check                 ",
    "Illegal opcode               ",
    "Device not available         ",
    "Double fault                 ",
    "Reserved (not used since 486)",
    "Invalid task switch segment  ",
    "Segment not present          ",
    "Stack exception              ",
    "General protection fault     ",
    "Page fault                   ",
    "Reserved                     ",
    "Floating point error         ",
    "Aligment check               ",
    "Machine check                ",
    "SIMD floating point error    ",
    "Interrupts                   ",
    "System call                  ",
    "Catchall                     ",
};

/**
 * @brief Map every trapno to an array index.
 * 
 * @param trapno The actual trapno
 * @return int The trap index
 */
static int trapnos(int trapno) {
    if (trapno >= 0 && trapno <= 19) // processor defined
        return trapno;
    if (trapno >= 32 && trapno < 64) // interrupts
        return 20;
    if (trapno == 64) // syscalls
        return 21;
    return 22; // default
};

/**
 * @brief Count print the syscalls that the current program called
 * 
 * @return int total number of syscalls
 */
static int countSyscalls(void) {
    struct proc *curproc = myproc();
    int s, i, c = 0;

    cprintf("\n-----[ Syscalls ]-----\n");

    for (i = 1; i < NELEM(curproc->syscounts); i++) {
        s = curproc->syscounts[i];
        if (s != 0) {
            cprintf(" %s\t : %d\n", sysname(i), s);
            c += s;
        }
    }
    cprintf("  Total Syscalls : %d\n", c);

    return c;
}

/**
 * @brief Print the count of each trap that occurred
 * 
 * @return int The total number of traps
 */
int sys_countTraps(void) {
    
    countSyscalls();

    struct proc *curproc = myproc();
    int s, i, c = 0;

    cprintf("\n-------------[ Traps ]--------------\n");

    for (i = 0; i < NELEM(curproc->trapcounts) - 1; i++) {
        s = curproc->trapcounts[i];
        if (s != 0) {
            cprintf(" %s : %d\n", trapnames[i], s);
            c += s;
        }
    }
    cprintf("  Total Traps                  : %d\n", c);
    return c;
}

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{

  // Update trap counter if process exists
  struct proc *curproc = myproc();
  if (curproc != 0)
      ++curproc->trapcounts[trapnos(tf->trapno)];

  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
