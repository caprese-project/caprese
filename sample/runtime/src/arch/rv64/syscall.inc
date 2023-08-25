#define DEFINE_SYSCALL(ns, f, nid, fid) \
  .global sys_ ## ns ## _ ## f         ;\
  .type sys_ ## ns ## _ ## f, @function;\
  sys_ ## ns ## _ ## f:                ;\
    li a7, (((nid) << 12) | (fid))     ;\
    ecall                              ;\
    ret                                ;
