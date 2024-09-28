#include "./syscall.h"
#include "string.h"

void itoa(int n, char *buf, int base) {
  int i = 0;
  int sign = n;
  if (sign < 0)
    n = -n;
  do {
    buf[i++] = "0123456789abcdef"[n % base];
  } while ((n /= base) > 0);
  if (sign < 0)
    buf[i++] = '-';
  buf[i] = '\0';
  for (int j = 0; j < i / 2; j++) {
    char tmp = buf[j];
    buf[j] = buf[i - j - 1];
    buf[i - j - 1] = tmp;
  }
}

__asm__(".text \n"
        ".global _start \n"
        "_start: \n"
        " lea 8(%rsp),%rdi \n"
        " jmp _main \n");

void _main(const char **argv) {
  int i, x = 0;
  char _buf[2048] = "envp[argv[";
  char *buf = _buf + 5;

  while (1) {
    for (i = 0; argv[x + i]; i++) {
      sys_write(1, "envp[", 5);
      char buf[10];
      itoa(i, buf, 10);
      sys_write(1, buf, strlen(buf));
      sys_write(1, "] = ", 4);
      sys_write(1, argv[x + i], strlen(argv[x + i]));
      sys_write(1, "\n", 1);
    }
    if (x > 0)
      break;
    x = i + 1;
  }
  sys_exit(0);
}