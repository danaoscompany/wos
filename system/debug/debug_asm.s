/*
 * x86_64/dbga.S
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
 * @brief Assembly routines for the mini debugger
 */

.section .text

/**
 * Set up dummy exception handlers to call the mini debugger
 */
.global dbg_init
.type dbg_init, @function
dbg_init:
    lea    dbg_idt, %esi
    lea    dbg_isrs, %edi
    /* exception gates */
    movw    $32, %cx
    xor    %eax, %eax
    movw    %cs, %ax
    or     $0x0E000000, %eax
    shl    $16, %eax
    movl    %eax, %edx
1:  movl    %edi, %eax
    andl    $0xFFFF0000, %eax
    shl    $48, %eax
    movw    %di, %ax
    or     %edx, %eax
    stosl
    movl    %edi, %eax
    shr    $32, %eax
    stosl
    addl    $32, %edi
    decw    %cx
    jnz     1b
    /* interrupt gates */
    xor    %eax, %eax
    movw    $16, %cx
    or     $0x0F000000, %eax
    shl    $16, %eax
    or     %eax, %edx
1:  movl    %edi, %eax
    andl    $0xFFFF0000, %eax
    shl    $48, %eax
    movw    %di, %ax
    or     %edx, %eax
    stosl
    movl    %edi, %eax
    shr    $32, %eax
    stosl
    addl    $32, %edi
    decw    %cx
    jnz     1b
    lidt    dbg_idt_value
    ret

.size dbg_init, . - dbg_init

/**
 * Save registers before we call any C code
 */
.global dbg_saveregs
dbg_saveregs:
	.extern dbg_regs
    movl    %eax, dbg_regs +  0
    movl    %ebx, dbg_regs +  8
    movl    %ecx, dbg_regs + 16
    movl    %edx, dbg_regs + 24
    movl    %esi, dbg_regs + 32
    movl    %edi, dbg_regs + 40
    movl    40(%esp), %eax          /* orig esp */
    movl    %eax, dbg_regs + 48
    movl    %eax, dbg_regs + 29*8
    movl    %ebp, dbg_regs + 56
    pushl %edi
    pushl %eax
    movl dbg_regs, %edi
    movl    r8, %eax
    movl %eax, 64(%edi)
    movl    r9, %eax
    movl %eax, 72(%edi)
    movl    r10, %eax
    movl %eax, 80(%edi)
    movl    r11, %eax
    movl %eax, 88(%edi)
    movl    r12, %eax
    movl %eax, 96(%edi)
    movl    r13, %eax
    movl %eax, 104(%edi)
    movl    r14, %eax
    movl %eax, 112(%edi)
    movl    r15, %eax
    movl %eax, 120(%edi)
    popl %eax
    popl %edi
    movl    16(%esp), %eax          /* orig eip */
    movl    %eax, dbg_regs + 128
    movl    %eax, dbg_regs + 31*8
    movl    %cr0, %eax
    movl    %eax, dbg_regs + 136
    movl    %cr1, %eax
    movl    %eax, dbg_regs + 144
    movl    %cr2, %eax
    movl    %eax, dbg_regs + 152
    movl    %cr3, %eax
    movl    %eax, dbg_regs + 160
    movl    %cr4, %eax
    movl    %eax, dbg_regs + 168
    movl    32(%esp), %eax          /* rflags */
    movl    %eax, dbg_regs + 176
    movl    8(%esp), %eax           /* exception code */
    movl    %eax, dbg_regs + 184
    ret

/**
 * Restore registers after we call C code
 */
.global dbg_loadregs
dbg_loadregs:
	.extern dbg_regs
    movl    dbg_regs +  0, %eax
    movl    dbg_regs +  8, %ebx
    movl    dbg_regs + 16, %ecx
    movl    dbg_regs + 24, %edx
    movl    dbg_regs + 32, %esi
    movl    dbg_regs + 40, %edi
    movl    dbg_regs + 56, %ebp
    pushl %edi
    pushl %eax
    movl dbg_regs, %edi
    movl 64(%edi), %eax
    movl %eax, r8
    movl 72(%edi), %eax
    movl %eax, r9
    movl 80(%edi), %eax
    movl %eax, r10
    movl 88(%edi), %eax
    movl %eax, r11
    movl 96(%edi), %eax
    movl %eax, r12
    movl 104(%edi), %eax
    movl %eax, r13
    movl 112(%edi), %eax
    movl %eax, r14
    movl 120(%edi), %eax
    movl %eax, r15
    popl %eax
    popl %edi
    ret

.macro isr num, errcode, name, func
    .balign 32
    cli
.if \errcode == 0
    pushl   $0
.endif
	.global dbg_saveregs
    call    dbg_saveregs
    addl    $8, %esp
    movl    $\num, %edi
    .extern dbg_main
    call    dbg_main
    movl    dbg_regs + 31*8, %eax   /* update the return address */
    movl    %eax, (%esp)
    .global dbg_loadregs
    call    dbg_loadregs
    iretl
.endm

    .balign 32
dbg_isrs:
    isr  0, 0
    isr  1, 0
    isr  2, 0
    isr  3, 0
    isr  4, 0
    isr  5, 0
    isr  6, 0
    isr  7, 0
    isr  8, 1
    isr  9, 0
    isr 10, 1
    isr 11, 1
    isr 12, 1
    isr 13, 1
    isr 14, 1
    isr 15, 0
    isr 16, 0
    isr 17, 1
    isr 18, 0
    isr 19, 0
    isr 20, 0
    isr 21, 0
    isr 22, 0
    isr 23, 0
    isr 24, 0
    isr 25, 0
    isr 26, 0
    isr 27, 0
    isr 28, 0
    isr 29, 0
    isr 30, 1
    isr 31, 0

    isr 32, 0
    isr 33, 0
    isr 34, 0
    isr 35, 0
    isr 36, 0
    isr 37, 0
    isr 38, 0
    isr 39, 0
    isr 40, 0
    isr 41, 0
    isr 42, 0
    isr 43, 0
    isr 44, 0
    isr 45, 0
    isr 46, 0
    isr 47, 0

.data
    .balign 16
r8: .long 0
r9: .long 0
r10: .long 0
r11: .long 0
r12: .long 0
r13: .long 0
r14: .long 0
r15: .long 0
dbg_idt:
    .space (48*16)
dbg_idt_value:
    .word (dbg_idt_value-dbg_idt)-1
    .long dbg_idt
