	.file	"struct.c"
	.section	.rodata
.LC0:
	.string	"hello"
.LC1:
	.string	"test"
.LC2:
	.string	"test1"
.LC3:
	.string	"ahh"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	$.LC0, -32(%rbp)
	movq	$.LC1, -24(%rbp)
	movl	$16, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	$.LC2, (%rax)
	movq	-8(%rbp), %rax
	movq	$.LC3, 8(%rax)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	free
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (GNU) 4.7.0 20120505 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
