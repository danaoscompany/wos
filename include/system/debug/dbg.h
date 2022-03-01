/*
 * x86_64/dbg.h
 *
 * Copyright (C) 2021 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @brief Mini debugger
 */

/**
 * Initialize exceptions
 */
#ifndef WOS_DEBUG_H
#define WOS_DEBUG_H
extern "C" void dbg_init();
extern "C" void dbg_printf(char *fmt, ...);

/**
 * Save registers
 */
extern "C" void dbg_saveregs();

/**
 * Invoke the debugger
 */
extern "C" void dbg_main(unsigned long excnum);
extern "C" void dbg_test();

/**
 * Restore registers
 */
extern void dbg_loadregs();

/**
 * Trigger an exception in code
 */
#define breakpoint __asm__ __volatile__("int $3")
#endif
