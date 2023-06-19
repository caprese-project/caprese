#include <cstdint>

#include <caprese/arch/riscv/panic.h>
#include <caprese/lib/console.h>

namespace caprese::arch {
  void dump_context() {
    uintptr_t ra, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2, a3, a4, a5, a6, a7, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, t3, t4, t5, t6;
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
    println("ra:  %p", ra);
    println("sp:  %p", sp);
    println("gp:  %p", gp);
    println("tp:  %p", tp);
    println("t0:  %p", t0);
    println("t1:  %p", t1);
    println("t2:  %p", t2);
    println("s0:  %p", s0);
    println("s1:  %p", s1);
    println("a0:  %p", a0);
    println("a1:  %p", a1);
    println("a2:  %p", a2);
    println("a3:  %p", a3);
    println("a4:  %p", a4);
    println("a5:  %p", a5);
    println("a6:  %p", a6);
    println("a7:  %p", a7);
    println("s2:  %p", s2);
    println("s3:  %p", s3);
    println("s4:  %p", s4);
    println("s5:  %p", s5);
    println("s6:  %p", s6);
    println("s7:  %p", s7);
    println("s8:  %p", s8);
    println("s9:  %p", s9);
    println("s10: %p", s10);
    println("s11: %p", s11);
    println("t3:  %p", t3);
    println("t4:  %p", t4);
    println("t5:  %p", t5);
    println("t6:  %p", t6);
  }

  [[noreturn]] void halt() {
    while (true) {
      asm volatile("wfi");
    }
  }
} // namespace caprese::arch
