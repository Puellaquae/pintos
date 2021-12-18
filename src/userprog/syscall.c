#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#define MAX_SYSCALLS 20

#define DECLARE_SYSCALL(name, func) syscall_desc_t name##_syscall_desc = {#name, &func}; syscalls[name] = name##_syscall_desc 

static void syscall_handler (struct intr_frame *);

typedef void handler_t(struct intr_frame *);

handler_t sys_halt;
handler_t sys_exit;
handler_t sys_exec;
handler_t sys_wait;
handler_t sys_create;
handler_t sys_remove;
handler_t sys_open;
handler_t sys_filesize;
handler_t sys_read;
handler_t sys_write;
handler_t sys_seek;
handler_t sys_tell;
handler_t sys_close;

typedef struct syscall_desc {
  char* name;
  handler_t* handler;
} syscall_desc_t;

syscall_desc_t syscalls[MAX_SYSCALLS] = {0};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  DECLARE_SYSCALL(SYS_HALT, sys_halt);
  DECLARE_SYSCALL(SYS_EXIT, sys_exit);
  DECLARE_SYSCALL(SYS_EXEC, sys_exec);
  DECLARE_SYSCALL(SYS_WAIT, sys_wait);
  DECLARE_SYSCALL(SYS_CREATE, sys_create);
  DECLARE_SYSCALL(SYS_REMOVE, sys_remove);
  DECLARE_SYSCALL(SYS_OPEN, sys_open);
  DECLARE_SYSCALL(SYS_WRITE, sys_write);
  DECLARE_SYSCALL(SYS_SEEK, sys_seek);
  DECLARE_SYSCALL(SYS_TELL, sys_tell);
  DECLARE_SYSCALL(SYS_CLOSE, sys_close);
  DECLARE_SYSCALL(SYS_READ, sys_read);
  DECLARE_SYSCALL(SYS_FILESIZE, sys_filesize);
}

void
syscall_nofound (void)
{
  thread_current ()->exit_code = -1;
  thread_exit ();
}

static void
syscall_handler (struct intr_frame *f) 
{
  int type = *(int *)f->esp;
  if (type < 0 || type >= MAX_SYSCALLS || syscalls[type].name == NULL)
    {
      syscall_nofound ();
    }
  syscalls[type].handler(f);
}

void 
sys_halt (struct intr_frame *f)
{
  shutdown_power_off();
}

void 
sys_exit (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int status = *(++ptr);

  thread_current ()->exit_code = status;
  thread_exit ();
}

void 
sys_exec (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *cmd_line = *(++ptr);
  int ret_val;

  f->eax = ret_val;
}

void 
sys_wait (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  tid_t tid = *(++ptr);
  int ret_val;

  f->eax = ret_val;
}

void 
sys_create (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *file = *(++ptr);
  unsigned inital_size = *(++ptr);
  bool ret_val;

  f->eax = ret_val;
}

void 
sys_remove (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *file = *(++ptr);
  bool ret_val;

  f->eax = ret_val;
}

void 
sys_open (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *file = *(++ptr);
  int ret_val;

  f->eax = ret_val;
}

void 
sys_filesize (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(++ptr);
  int ret_val;

  f->eax = ret_val;
}

void 
sys_read (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(++ptr);
  char *buffer = *(++ptr);
  unsigned size = *(++ptr);
  int ret_val;

  f->eax = ret_val;
}

void 
sys_write (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(++ptr);
  char *buffer = *(++ptr);
  unsigned size = *(++ptr);
  int ret_val;

  if (fd == 1)
    {
      putbuf (buffer, size);
      ret_val = size;
    }
  else
    {
      ret_val = -1;
    }

  f->eax = ret_val;
}

void 
sys_seek (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(++ptr);
  unsigned position = *(++ptr);
  int ret_val;

  f->eax = ret_val;
}

void 
sys_tell (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(++ptr);
  unsigned ret_val;

  f->eax = ret_val;
}

void 
sys_close (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(++ptr);

}