#include <cstdio>

#include <caprese/arch/rv64/panic.h>

namespace caprese::arch {
  void dump_context() {
    void *ra, *sp, *gp, *tp, *t0, *t1, *t2, *s0, *s1, *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *t3, *t4, *t5,
        *t6;
    asm volatile("mv %0, ra" : "=r"(ra));
    asm volatile("mv %0, sp" : "=r"(sp));
    asm volatile("mv %0, gp" : "=r"(gp));
    asm volatile("mv %0, tp" : "=r"(tp));
    asm volatile("mv %0, t0" : "=r"(t0));
    asm volatile("mv %0, t1" : "=r"(t1));
    asm volatile("mv %0, t2" : "=r"(t2));
    asm volatile("mv %0, s0" : "=r"(s0));
    asm volatile("mv %0, s1" : "=r"(s1));
    asm volatile("mv %0, a0" : "=r"(a0));
    asm volatile("mv %0, a1" : "=r"(a1));
    asm volatile("mv %0, a2" : "=r"(a2));
    asm volatile("mv %0, a3" : "=r"(a3));
    asm volatile("mv %0, a4" : "=r"(a4));
    asm volatile("mv %0, a5" : "=r"(a5));
    asm volatile("mv %0, a6" : "=r"(a6));
    asm volatile("mv %0, a7" : "=r"(a7));
    asm volatile("mv %0, s2" : "=r"(s2));
    asm volatile("mv %0, s3" : "=r"(s3));
    asm volatile("mv %0, s4" : "=r"(s4));
    asm volatile("mv %0, s5" : "=r"(s5));
    asm volatile("mv %0, s6" : "=r"(s6));
    asm volatile("mv %0, s7" : "=r"(s7));
    asm volatile("mv %0, s8" : "=r"(s8));
    asm volatile("mv %0, s9" : "=r"(s9));
    asm volatile("mv %0, s10" : "=r"(s10));
    asm volatile("mv %0, s11" : "=r"(s11));
    asm volatile("mv %0, t3" : "=r"(t3));
    asm volatile("mv %0, t4" : "=r"(t4));
    asm volatile("mv %0, t5" : "=r"(t5));
    asm volatile("mv %0, t6" : "=r"(t6));
    printf("ra:  %p\n", ra);
    printf("sp:  %p\n", sp);
    printf("gp:  %p\n", gp);
    printf("tp:  %p\n", tp);
    printf("t0:  %p\n", t0);
    printf("t1:  %p\n", t1);
    printf("t2:  %p\n", t2);
    printf("s0:  %p\n", s0);
    printf("s1:  %p\n", s1);
    printf("a0:  %p\n", a0);
    printf("a1:  %p\n", a1);
    printf("a2:  %p\n", a2);
    printf("a3:  %p\n", a3);
    printf("a4:  %p\n", a4);
    printf("a5:  %p\n", a5);
    printf("a6:  %p\n", a6);
    printf("a7:  %p\n", a7);
    printf("s2:  %p\n", s2);
    printf("s3:  %p\n", s3);
    printf("s4:  %p\n", s4);
    printf("s5:  %p\n", s5);
    printf("s6:  %p\n", s6);
    printf("s7:  %p\n", s7);
    printf("s8:  %p\n", s8);
    printf("s9:  %p\n", s9);
    printf("s10: %p\n", s10);
    printf("s11: %p\n", s11);
    printf("t3:  %p\n", t3);
    printf("t4:  %p\n", t4);
    printf("t5:  %p\n", t5);
    printf("t6:  %p\n", t6);
  }

  [[noreturn]] void halt() {
    while (true) {
      asm volatile("wfi");
    }
  }
} // namespace caprese::arch
