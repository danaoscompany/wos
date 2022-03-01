#ifndef ISR_H
#define ISR_H

struct regs_t {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

typedef regs_t Register;

void init_isr();
void isr_set_handler(int isr, void (*handler)(Register* regs));
void isr_remove_handler(int isr);

#endif
