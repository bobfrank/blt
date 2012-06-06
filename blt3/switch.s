	.file	"switch.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"0"
.LC1:
	.string	"one"
.LC2:
	.string	"second"
.LC3:
	.string	"third"
.LC4:
	.string	"four"
.LC5:
	.string	"five"
	.text
	.globl	main
	.type	main, @function
main:
.LFB11:
	.cfi_startproc
	cmpl	$5, %edi
	ja	.L13
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	%edi, %edi
	jmp	*.L9(,%rdi,8)
	.section	.rodata
	.align 8
	.align 4
.L9:
	.quad	.L3
	.quad	.L4
	.quad	.L5
	.quad	.L6
	.quad	.L7
	.quad	.L8
	.text
.L3:
	movl	$.LC0, %edi
	call	puts
	jmp	.L10
.L4:
	movl	$.LC1, %edi
	call	puts
	.p2align 4,,3
	jmp	.L10
.L5:
	movl	$.LC2, %edi
	call	puts
	.p2align 4,,3
	jmp	.L10
.L6:
	movl	$.LC3, %edi
	call	puts
	.p2align 4,,3
	jmp	.L10
.L7:
	movl	$.LC4, %edi
	call	puts
	.p2align 4,,3
	jmp	.L10
.L8:
	movl	$.LC5, %edi
	call	puts
.L10:
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
.L13:
	rep
	ret
	.cfi_endproc
.LFE11:
	.size	main, .-main
	.ident	"GCC: (GNU) 4.7.0 20120505 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
