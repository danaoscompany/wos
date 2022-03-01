#ifndef LIBSYSTEM_H
#define LIBSYSTEM_H

#include <libgraphics.h>
#include <libresources.h>
#include <libstring.h>

void danaos_init_all();
void danaos_deinit();
void call(int eax, int ebx, int ecx, int edx);

#endif
