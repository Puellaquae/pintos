#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "fixed_point.h"
#include "threads/synch.h"

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */


#ifdef USERPROG
/* Thread info used in parent thread's children list.
   If store in thread, it will be freed after thread exit. */
struct child_info
  {
    tid_t tid;
    struct list_elem childelem;   /* Used in parent thread's children list. */
    int exit_code;
    struct thread *thread;
    struct semaphore sema;
  };

struct file_elem
  {
    int fd;
    struct file *file;
    struct list_elem elem;
  };
#endif

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    int origin_priority;                /* Origin priority which not got by priority donation. */

    struct list_elem allelem;           /* List element for all threads list. */
    struct list_elem waitelem;          /* List element for wait threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element in ready list or sema waiters. */

    struct list locks;                  /* The locks list for this thread holds. */
    struct lock *waiting_lock;          /* The lock which this thread waiting for. */

    int64_t sleep_ticks;                /* Ticks when awake */

    int nice;
    fixed_t recent_cpu;

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
    struct thread *parent;              /* The thread of who create this thread. */
    struct list children;               /* The thread that this thread created. */
    struct child_info *child_info;      /* Child thread info for parent thread use. */
    int exit_code;                      /* Thread exit code, -1 for init. */
    struct semaphore sema;              /* Semaphore for child process to block parent until program loaded. */
    int child_success;                  /* Keep whether child process is start successfully;. */
    struct list files;                  /* List of opened files. */
    int fd_counter;                     /* For get next fd. */
    struct file *self_exec_file;        /* Its executable file */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

#ifdef USERPROG
void acquire_file_lock (void);
void release_file_lock (void);
#endif

void thread_tick (int64_t ticks);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

void thread_sleep_until (int64_t wake_tick);
void thread_wake (int64_t ticks);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);
void thread_priority_donate (struct thread *t, int new_priority);
bool thread_priority_greater_compare (const struct list_elem *a, const struct list_elem *b, void *aux);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);
void thread_mlfqs_update_priority (struct thread *t);
void thread_mlfqs_increase_recent_cpu (void);
void thread_mlfqs_update_load_avg_and_recent_cpu (void);

#endif /* threads/thread.h */
