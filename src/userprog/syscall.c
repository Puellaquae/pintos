#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

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

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

void 
is_vaild_user_ptr (void *ptr)
{
  if (!is_user_vaddr (ptr))
    {
      syscall_fail ();
    }

  void *pptr = pagedir_get_page (thread_current ()->pagedir, ptr);
  if (!pptr)
    {
      syscall_fail ();
    }
}

/* Return origin ptr or exit (-1) if invaild. */
void *
check_ptr (void *ptr)
{
  uint8_t *check_bptr = ptr;
  for (int i = 0; i < sizeof (void *); i++)
    {
      is_vaild_user_ptr (check_bptr + i);
      if (get_user (check_bptr + i) == -1)
        {
          syscall_fail ();
        }
    }
  return ptr;
}

void
syscall_fail (void)
{
  thread_current ()->exit_code = -1;
  thread_exit ();
}

static void
syscall_handler (struct intr_frame *f) 
{
  int type = *(int *)check_ptr (f->esp);
  if (type < 0 || type >= MAX_SYSCALLS || syscalls[type].name == NULL)
    {
      syscall_fail ();
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
  int status = *(int *)check_ptr (++ptr);

  thread_current ()->exit_code = status;
  thread_exit ();
}

void 
sys_exec (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *cmd_line = *(char **)check_ptr (++ptr);
  check_ptr (cmd_line);
  tid_t ret_val;

  ret_val = process_execute (cmd_line);

  f->eax = ret_val;
}

void 
sys_wait (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  tid_t tid = *(tid_t *)check_ptr (++ptr);
  int ret_val;

  ret_val = process_wait (tid);
  
  f->eax = ret_val;
}

struct file_elem*
get_find_by_fd (int fd)
{
  struct list *files = &(thread_current ()->files);
  for (struct list_elem *e = list_begin (files); e != list_end (files); e = list_next (e))
    {
      struct file_elem *file_elem = list_entry (e, struct file_elem, elem);
      if (file_elem->fd == fd)
        {
          return file_elem;
        }
    }

  return NULL;
}

void 
sys_create (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *file = *(char **)check_ptr (++ptr);
  check_ptr (file);
  unsigned inital_size = *(++ptr);
  bool ret_val;

  acquire_file_lock ();
  ret_val = filesys_create (file, inital_size);
  release_file_lock ();

  f->eax = ret_val;
}

void 
sys_remove (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *file = *(char **)check_ptr (++ptr);
  check_ptr (file);
  bool ret_val;

  acquire_file_lock ();
  ret_val = filesys_remove (file);
  release_file_lock ();

  f->eax = ret_val;
}

void 
sys_open (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  char *file = *(char **)check_ptr (++ptr);
  check_ptr (file);
  int ret_val = -1;

  acquire_file_lock ();
  struct file *file_ptr = filesys_open (file);
  release_file_lock ();
  if (file_ptr != NULL)
    {
      struct file_elem *file_elem = malloc (sizeof (struct file_elem));
      ret_val = file_elem->fd = thread_current ()->fd_counter++;
      file_elem->file = file_ptr;
      list_push_back (&thread_current ()->files, &file_elem->elem);
    }

  f->eax = ret_val;
}

void 
sys_filesize (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(int *)check_ptr (++ptr);
  int ret_val = -1;

  struct file_elem* file_elem = get_find_by_fd (fd);
  if (file_elem != NULL)
    {
      acquire_file_lock ();
      ret_val = file_length (file_elem->file);
      release_file_lock ();
    }

  f->eax = ret_val;
}

void 
sys_read (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(int *)check_ptr (++ptr);
  char *buffer = *(char **)check_ptr (++ptr);
  check_ptr (buffer);
  unsigned size = *(unsigned *)check_ptr (++ptr);
  int ret_val = -1;

  if (fd == 0)
    {
      for (int i = 0; i < size; i++)
        {
          buffer[i] = input_getc ();
        }
      ret_val = size;
    }
  else 
    {
      struct file_elem *file_elem = get_find_by_fd (fd);
      if (file_elem != NULL)
        {
          acquire_file_lock ();
          ret_val = file_read (file_elem->file, buffer, size);
          release_file_lock ();
        }
    }

  f->eax = ret_val;
}

void 
sys_write (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(int *)check_ptr (++ptr);
  char *buffer = *(char **)check_ptr (++ptr);
  check_ptr (buffer);
  unsigned size = *(unsigned *)check_ptr (++ptr);
  int ret_val = 0;

  if (fd == 1)
    {
      putbuf (buffer, size);
      ret_val = size;
    }
  else
    {
      struct file_elem *file_elem = get_find_by_fd (fd);
      if (file_elem != NULL)
        {
          acquire_file_lock ();
          ret_val = file_write (file_elem->file, buffer, size);
          release_file_lock ();
        }
    }

  f->eax = ret_val;
}

void 
sys_seek (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(int *)check_ptr (++ptr);
  unsigned position = *(unsigned *)check_ptr (++ptr);

  struct file_elem *file_elem = get_find_by_fd (fd);
  if (file_elem != NULL)
    {
      acquire_file_lock ();
      file_seek (file_elem->file, position);
      release_file_lock ();
    }
}

void 
sys_tell (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(int *)check_ptr (++ptr);
  unsigned ret_val;

  struct file_elem *file_elem = get_find_by_fd (fd);
  if (file_elem != NULL)
    {
      acquire_file_lock ();
      ret_val = file_tell (file_elem->file);
      release_file_lock ();
    }
  
  f->eax = ret_val;
}

void 
sys_close (struct intr_frame *f)
{
  uint32_t *ptr = f->esp;
  int fd = *(int *)check_ptr (++ptr);

  struct file_elem *file_elem = get_find_by_fd (fd);
  if (file_elem != NULL)
    {
      acquire_file_lock ();
      file_close (file_elem->file);
      release_file_lock ();

      list_remove (&file_elem->elem);
      free (file_elem);
    }
}