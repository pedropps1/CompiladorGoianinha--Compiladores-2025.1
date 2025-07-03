.data
nl: .asciiz "
"
str2: .asciiz " passei do j!"
str1: .asciiz " achei o j!"
str0: .asciiz "i = "

.text

.globl main
.text
main:
programa:
    addi $sp, $sp, -16
    sw   $ra, 0($sp)
    sw   $fp, 4($sp)
    move $fp, $sp
    li $t0, 0
    sw $t0, -8($fp)
    li $t0, 3
    sw $t0, -12($fp)
L1:
    lw $t0, -8($fp)
    li $t1, 5
    slt $t0, $t0, $t1
    beq $t0, $zero, L2
    la $t0, str0
    move $a0, $t0
    li $v0, 4
    syscall
    lw $t0, -8($fp)
    move $a0, $t0
    li $v0, 1
    syscall
    la $a0, nl
    li $v0, 4
    syscall
    lw $t0, -8($fp)
    lw $t1, -12($fp)
    seq $t0, $t0, $t1
    beq $t0, $zero, L3
    la $t0, str1
    move $a0, $t0
    li $v0, 4
    syscall
    la $a0, nl
    li $v0, 4
    syscall
    j L4
L3:
    lw $t0, -8($fp)
    lw $t1, -12($fp)
    sgt $t0, $t0, $t1
    beq $t0, $zero, L5
    la $t0, str2
    move $a0, $t0
    li $v0, 4
    syscall
    la $a0, nl
    li $v0, 4
    syscall
L5:
L4:
    lw $t0, -8($fp)
    li $t1, 1
    add $t0, $t0, $t1
    sw $t0, -8($fp)
    j L1
L2:
L0:
    # Epílogo
    lw $ra, 0($sp)
    lw $fp, 4($sp)
    addi $sp, $sp, 16
    jr $ra
