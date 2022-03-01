#ifndef MACHINE_H
#define MACHINE_H

void cpuid(int code);
int read_cr(int number);
void write_cr(int number, int value);
void read_msr(int code);
void write_msr(int code, int eax, int edx);
extern int cpuid_eax;
extern int cpuid_edx;
extern int msr_eax;
extern int msr_ecx;
extern int msr_edx;
void wait();
void stop();
void loop();
void ack();
void log(char *message, char** args);

#endif
