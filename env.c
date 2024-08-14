void itoa(int num, char **str) {
  char buf[4];
  int i = 0;
  while (num) {
    buf[i++] = num % 10 + '0';
    num /= 10;
  }
  if (i == 0)
    buf[i++] = '0';
  while (i) {
    *(*str)++ = buf[--i];
  }
}

void strcat(char **dst, const char *src) {
  while (*src)
    *(*dst)++ = *src++;
}

void output(const char *str, int len) {
  __asm__(" mov $1,%%rax \n"
          " mov $1,%%rdi \n"
          " mov %0,%%rsi \n"
          " mov %1,%%edx \n"
          " syscall \n"
          :
          : "r"(str), "r"(len)
          : "rax", "rdi", "rsi", "rdx", "rcx", "r11");
}

__asm__(".text \n"
        ".global _start \n"
        "_start: \n"
        " lea 8(%rsp),%rdi \n"
        " jmp _main \n");

[[noreturn]] void _main(const char **argv) {

  int i, x = 0;
  char _buf[2048] = "envp[argv[";
  char *buf = _buf + 5;

  for (;;) {
    for (i = 0; argv[x + i]; i++) {
      char *p = buf + 5;
      itoa(i, &p);
      *p++ = ']';
      *p++ = ' ';
      *p++ = '=';
      *p++ = ' ';
      strcat(&p, argv[x + i]);
      *p++ = '\n';
      *p = 0;
      output(buf, p - buf);
    }
    if (x)
      break;
    buf -= 5;
    x = i + 1;
  }
  // syscall exit(status)
  [[noreturn]] __asm__(" mov $0x3c,%%rax \n"
                       " mov $0,%%edi \n"
                       " syscall \n"
                       :
                       :
                       : "rax", "rdi");
}