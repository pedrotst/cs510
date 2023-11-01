	.file	"test1.c"
	.text
	.globl	fd1
	.bss
	.align 4
	.type	fd1, @object
	.size	fd1, 4
fd1:
	.zero	4
	.globl	fd2
	.align 4
	.type	fd2, @object
	.size	fd2, 4
fd2:
	.zero	4
	.globl	c
	.type	c, @object
	.size	c, 1
c:
	.zero	1
	.section	.rodata
.LC0:
	.string	"foo.txt"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$0, %esi
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	open@PLT
	movl	%eax, fd1(%rip)
	movl	fd1(%rip), %eax
	movl	$1, %edx
	leaq	c(%rip), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	read@PLT
	movl	fd2(%rip), %eax
	movl	$1, %edx
	leaq	c(%rip), %rcx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	write@PLT
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
