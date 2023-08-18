#define DEFINE_SYSCALL(ns, f, nid, fid) \
  .global sys_ ## ns ## _ ## f         ;\
  .type sys_ ## ns ## _ ## f, @function;\
  sys_ ## ns ## _ ## f:                ;\
    li a7, (((nid) << 12) | (fid))     ;\
    ecall                              ;\
    ret                                ;

.section .text

DEFINE_SYSCALL(base, null, 0, 0)
DEFINE_SYSCALL(base, core_id, 0, 1)

DEFINE_SYSCALL(debug, putchar, 1, 0)

DEFINE_SYSCALL(cap, call_method, 2, 2)
DEFINE_SYSCALL(cap, get_field, 2, 3)
DEFINE_SYSCALL(cap, is_permitted, 2, 4)
DEFINE_SYSCALL(cap, list_size, 2, 5)
