#include <debug-uart.h>
#include <sys/stat.h>
#include <errno.h>

int
_open(const char *name, int flags, int mode) {
  errno = ENOENT;
  return -1;
}

int
_close(int file)
{
  if (file == 1 || file == 2) {
    dbg_drain();
    return 0;
  }
  errno = EBADF;
  return -1;
}

int
isatty(int file)
{
  if (file >= 0 && file <= 2) return 1;
   return 0;
}

int
_read(int file, char *ptr, int len){
  return 0;
}

 	

int
_write(int file, const char *ptr, int len){
  int sent = -1;
  if (file == 1 || file == 2) {
    sent = dbg_send_bytes((const unsigned char*)ptr, len);
  }
  return sent;
}

int
_lseek(int file, int ptr, int dir){
    return 0;
}

int
_fstat(int file, struct stat *st) {
  if (file >= 0 && file <= 2) {
    st->st_mode = S_IFCHR;
    return 0;
  }
  errno = EBADF;
  return -1;
}

int
_stat(char *file, struct stat *st) {
  errno = ENOENT;
  return -1;
}

caddr_t
_sbrk(int incr)
{
  extern char __heap_start__;		/* Defined by the linker */
  extern char __heap_end__;		/* Defined by the linker */
  static char *heap_end;
  char *prev_heap_end;
 
  if (heap_end == 0) {
    heap_end = &__heap_start__;
  }
  prev_heap_end = heap_end;
  if (heap_end + incr > &__heap_end__) {
    _write (2, "Heap full\n", 10);
    errno = ENOMEM;
    return (caddr_t)-1;
  }

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int
fsync(int fd)
{
  if (fd == 1 || fd == 2) {
    dbg_drain();
    return 0;
  }
  if (fd == 0) return 0;
  errno = EBADF;
  return -1;
}
