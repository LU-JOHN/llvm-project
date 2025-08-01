// RUN: llvm-mc -filetype=obj -triple i386 %s -o - | llvm-objdump -d --no-show-raw-insn - | FileCheck %s

// Layout is optimial only if .align is based on current layout offset.

// CHECK:       cc:  nop
// CHECK-NEXT:  cd:  int3
// CHECK-NEXT:  ce:  int3
// CHECK-NEXT:  cf:  int3	
// CHECK:       d0:  pushal
// CHECK:      130:  jl      0xd0

L0:
        .space 0x8a, 0x90
	jmp	L0
        .space (0xb3 - 0x8f), 0x90
	jle	L2
        .space (0xcd - 0xb5), 0x90
	.p2align 4, 0xcc
L1:
        .space (0x130 - 0xd0),0x60
	jl	L1
L2:
