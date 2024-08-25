.section .initial_jump
.global _start

.align 4
_start:
    la sp, _sstack
    j main
