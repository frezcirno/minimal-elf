#pragma once
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

static inline long __syscall0(long n) {
  unsigned long ret;
  __asm__ __volatile__("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
  return ret;
}

static inline long __syscall1(long n, long a1) {
  unsigned long ret;
  __asm__ __volatile__("syscall"
                       : "=a"(ret)
                       : "a"(n), "D"(a1)
                       : "rcx", "r11", "memory");
  return ret;
}

static inline long __syscall2(long n, long a1, long a2) {
  unsigned long ret;
  __asm__ __volatile__("syscall"
                       : "=a"(ret)
                       : "a"(n), "D"(a1), "S"(a2)
                       : "rcx", "r11", "memory");
  return ret;
}

static inline long __syscall3(long n, long a1, long a2, long a3) {
  unsigned long ret;
  __asm__ __volatile__("syscall"
                       : "=a"(ret)
                       : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                       : "rcx", "r11", "memory");
  return ret;
}

static inline long __syscall4(long n, long a1, long a2, long a3, long a4) {
  unsigned long ret;
  register long r10 __asm__("r10") = a4;
  __asm__ __volatile__("syscall"
                       : "=a"(ret)
                       : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                       : "rcx", "r11", "memory");
  return ret;
}

static inline long __syscall5(long n, long a1, long a2, long a3, long a4,
                              long a5) {
  unsigned long ret;
  register long r10 __asm__("r10") = a4;
  register long r8 __asm__("r8") = a5;
  __asm__ __volatile__("syscall"
                       : "=a"(ret)
                       : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
                       : "rcx", "r11", "memory");
  return ret;
}

static inline long __syscall6(long n, long a1, long a2, long a3, long a4,
                              long a5, long a6) {
  unsigned long ret;
  register long r10 __asm__("r10") = a4;
  register long r8 __asm__("r8") = a5;
  register long r9 __asm__("r9") = a6;
  __asm__ __volatile__("syscall"
                       : "=a"(ret)
                       : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8),
                         "r"(r9)
                       : "rcx", "r11", "memory");
  return ret;
}

static inline int sys_open(const char *pathname, int flags, int mode) {
  return __syscall3(SYS_open, (long)pathname, flags, mode);
}

static inline int sys_read(int fd, char *buf, int n) {
  return __syscall3(SYS_read, fd, (long)buf, n);
}

static inline int sys_write(int fd, const char *buf, int n) {
  return __syscall3(SYS_write, fd, (long)buf, n);
}

static inline int sys_lseek(int fd, int off, int start) {
  return __syscall3(SYS_lseek, fd, off, start);
}

static inline int sys_close(int fd) { return __syscall1(SYS_close, fd); }

static inline int sys_access(const char *filename, int amode) {
  return __syscall2(SYS_access, (long)filename, amode);
}

static inline int sys_openat(int dirfd, const char *pathname, int flags,
                             int mode) {
  return __syscall4(SYS_openat, dirfd, (long)pathname, flags, mode);
}

static inline int sys_mkdir(const char *pathname, int mode) {
  return __syscall2(SYS_mkdir, (long)pathname, mode);
}

static inline int sys_mkdirat(int dirfd, const char *pathname, int mode) {
  return __syscall3(SYS_mkdirat, dirfd, (long)pathname, mode);
}

[[noreturn]] static inline void sys_exit(int r) {
  __syscall1(SYS_exit, r);
  __builtin_unreachable();
}

static inline ssize_t sys_copy_file_range(int fd_in, off_t *off_in, int fd_out,
                                          off_t *off_out, size_t len,
                                          unsigned flags) {
  return __syscall6(SYS_copy_file_range, fd_in, (long)off_in, fd_out,
                    (long)off_out, len, flags);
}

static inline void *sys_mmap(void *addr, size_t length, int prot, int flags,
                             int fd, off_t offset) {
  return (void *)__syscall6(SYS_mmap, (long)addr, length, prot, flags, fd,
                            offset);
}

static inline int sys_munmap(void *addr, size_t length) {
  return __syscall2(SYS_munmap, (long)addr, length);
}

static inline int sys_execve(const char *path, char *const argv[],
                             char *const envp[]) {
  /* do we need to use environ if envp is null? */
  return __syscall3(SYS_execve, (long)path, (long)argv, (long)envp);
}

static inline int sys_execveat(int dirfd, const char *pathname,
                               char *const argv[], char *const envp[],
                               int flags) {
  return __syscall5(SYS_execveat, dirfd, (long)pathname, (long)argv, (long)envp,
                    flags);
}

static inline void __procfdname(char *buf, unsigned fd) {
  unsigned i, j;
  for (i = 0; (buf[i] = "/proc/self/fd/"[i]); i++)
    ;
  for (j = fd; j; j /= 10, i++)
    ;
  buf[i] = 0;
  for (; fd; fd /= 10)
    buf[--i] = '0' + fd % 10;
}

static inline int sys_fcntl(int fd, int cmd) {
  return __syscall2(SYS_fcntl, fd, cmd);
}

static inline int sys_ftruncate(int fd, off_t length) {
  return __syscall2(SYS_ftruncate, fd, length);
}

static inline int sys_fchmod(int fd, mode_t mode) {
  int ret = __syscall2(SYS_fchmod, fd, mode);
  if (ret != -EBADF || sys_fcntl(fd, F_GETFD) < 0)
    return ret;

  char buf[15 + 3 * sizeof(int)];
  __procfdname(buf, fd);
#ifdef SYS_chmod
  return __syscall2(SYS_chmod, (long)buf, mode);
#else
  return __syscall3(SYS_fchmodat, AT_FDCWD, (long)buf, mode);
#endif
}

static inline int sys_fexecve(int fd, char *const argv[], char *const envp[]) {
  int r = sys_execveat(fd, "", argv, envp, AT_EMPTY_PATH);
  if (r != -ENOSYS)
    return r;
  char buf[15 + 3 * sizeof(int)];
  __procfdname(buf, fd);
  sys_execve(buf, argv, envp);
  return -1;
}

static inline int sys_fallocate(int fd, int mode, off_t base, off_t len) {
  return __syscall4(SYS_fallocate, fd, mode, base, len);
}

static inline int sys_ioctl(int fd, int req, void *arg) {
  return __syscall3(SYS_ioctl, fd, req, (long)arg);
}

static inline int sys_getdents(int fd, struct dirent *buf, size_t len) {
  return __syscall3(SYS_getdents, fd, (long)buf, len);
}

static inline int sys_memfd_create(const char *name, unsigned flags) {
  return __syscall2(SYS_memfd_create, (long)name, flags);
}
