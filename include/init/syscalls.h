#ifndef SYSCALLS_H
#define SYSCALLS_H

extern "C" void syscall();
extern "C" void syscall_handler(Register* regs);
void init_syscalls();
void call(int eax, int ebx, int ecx, int edx);

#endif
