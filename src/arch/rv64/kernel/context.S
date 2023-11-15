.section .text

/* void _switch_context(context_t* new_context, context_t* old_context) */
.global _switch_context
.type _switch_context, @function
/* void _load_context(context_t* context) */
.global _load_context
.type _load_context, @function

.balign 4
_switch_context:
  sd ra, 0(a1)
  sd sp, 8(a1)
  sd s0, 16(a1)
  sd s1, 24(a1)
  sd s2, 32(a1)
  sd s3, 40(a1)
  sd s4, 48(a1)
  sd s5, 56(a1)
  sd s6, 64(a1)
  sd s7, 72(a1)
  sd s8, 80(a1)
  sd s9, 88(a1)
  sd s10, 96(a1)
  sd s11, 104(a1)

.balign 4
_load_context:
  ld ra, 0(a0)
  ld sp, 8(a0)
  ld s0, 16(a0)
  ld s1, 24(a0)
  ld s2, 32(a0)
  ld s3, 40(a0)
  ld s4, 48(a0)
  ld s5, 56(a0)
  ld s6, 64(a0)
  ld s7, 72(a0)
  ld s8, 80(a0)
  ld s9, 88(a0)
  ld s10, 96(a0)
  ld s11, 104(a0)

  ret
