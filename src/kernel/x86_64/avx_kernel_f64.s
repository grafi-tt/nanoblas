/* ebp: k_len                     (escape to stack; moved from reg)         */
/* rsp: stack pointer                                                       */
/* rdi: b_pack                    (init. k_len)                             */
/* rsi: a_pack                                                              */
/* edx: k_len_sched               (loaded from stack; init. b_pack)         */
/* ecx: m_len_sched                                                         */
/* r8:  a                         (loaded from stack; init. k_len_next )    */
/* r9:  trans_a ? lda :   1       (init. trans_a, which loaded from stack)  */
/* r10: trans_a ?   1 : lda       (init. lda, which loaded from stack)      */
/* r11: a_pack_next               (loaded from stack)                       */
/* rax: tmp, at begin and end ldc (loaded from stack)                       */
/* rbx: tmp, at begin and end c   (escape to stack; loaded from stack)      */
/* r12: tmp                       (escape to stack)                         */
/* r13: reserved                                                            */
/* r14: reserved                                                            */
/* r15: reserved                                                            */

.section .text

.align 4
.globl avx_kernel_f64
avx_kernel_f64:
	ret /* dummy */
