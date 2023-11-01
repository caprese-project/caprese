#include <cstdint>

#include <log/log.h>

namespace {
  constexpr const char* tag = "log/dump";
} // namespace

void dump() {
  // general-purpose registers

  uint64_t x[32];

  asm volatile("sd x0, %0" : "=m"(x[0]));
  asm volatile("sd x1, %0" : "=m"(x[1]));
  asm volatile("sd x2, %0" : "=m"(x[2]));
  asm volatile("sd x3, %0" : "=m"(x[3]));
  asm volatile("sd x4, %0" : "=m"(x[4]));
  asm volatile("sd x5, %0" : "=m"(x[5]));
  asm volatile("sd x6, %0" : "=m"(x[6]));
  asm volatile("sd x7, %0" : "=m"(x[7]));
  asm volatile("sd x8, %0" : "=m"(x[8]));
  asm volatile("sd x9, %0" : "=m"(x[9]));
  asm volatile("sd x10, %0" : "=m"(x[10]));
  asm volatile("sd x11, %0" : "=m"(x[11]));
  asm volatile("sd x12, %0" : "=m"(x[12]));
  asm volatile("sd x13, %0" : "=m"(x[13]));
  asm volatile("sd x14, %0" : "=m"(x[14]));
  asm volatile("sd x15, %0" : "=m"(x[15]));
  asm volatile("sd x16, %0" : "=m"(x[16]));
  asm volatile("sd x17, %0" : "=m"(x[17]));
  asm volatile("sd x18, %0" : "=m"(x[18]));
  asm volatile("sd x19, %0" : "=m"(x[19]));
  asm volatile("sd x20, %0" : "=m"(x[20]));
  asm volatile("sd x21, %0" : "=m"(x[21]));
  asm volatile("sd x22, %0" : "=m"(x[22]));
  asm volatile("sd x23, %0" : "=m"(x[23]));
  asm volatile("sd x24, %0" : "=m"(x[24]));
  asm volatile("sd x25, %0" : "=m"(x[25]));
  asm volatile("sd x26, %0" : "=m"(x[26]));
  asm volatile("sd x27, %0" : "=m"(x[27]));
  asm volatile("sd x28, %0" : "=m"(x[28]));
  asm volatile("sd x29, %0" : "=m"(x[29]));
  asm volatile("sd x30, %0" : "=m"(x[30]));
  asm volatile("sd x31, %0" : "=m"(x[31]));

  logf(tag, "x0  (zero):   %p", x[0]);
  logf(tag, "x1  (ra):     %p", x[1]);
  logf(tag, "x2  (sp):     %p", x[2]);
  logf(tag, "x3  (gp):     %p", x[3]);
  logf(tag, "x4  (tp):     %p", x[4]);
  logf(tag, "x5  (t0):     %p", x[5]);
  logf(tag, "x6  (t1):     %p", x[6]);
  logf(tag, "x7  (t2):     %p", x[7]);
  logf(tag, "x8  (s0/fp):  %p", x[8]);
  logf(tag, "x9  (s1):     %p", x[9]);
  logf(tag, "x10 (a0):     %p", x[10]);
  logf(tag, "x11 (a1):     %p", x[11]);
  logf(tag, "x12 (a2):     %p", x[12]);
  logf(tag, "x13 (a3):     %p", x[13]);
  logf(tag, "x14 (a4):     %p", x[14]);
  logf(tag, "x15 (a5):     %p", x[15]);
  logf(tag, "x16 (a6):     %p", x[16]);
  logf(tag, "x17 (a7):     %p", x[17]);
  logf(tag, "x18 (s2):     %p", x[18]);
  logf(tag, "x19 (s3):     %p", x[19]);
  logf(tag, "x20 (s4):     %p", x[20]);
  logf(tag, "x21 (s5):     %p", x[21]);
  logf(tag, "x22 (s6):     %p", x[22]);
  logf(tag, "x23 (s7):     %p", x[23]);
  logf(tag, "x24 (s8):     %p", x[24]);
  logf(tag, "x25 (s9):     %p", x[25]);
  logf(tag, "x26 (s10):    %p", x[26]);
  logf(tag, "x27 (s11):    %p", x[27]);
  logf(tag, "x28 (t3):     %p", x[28]);
  logf(tag, "x29 (t4):     %p", x[29]);
  logf(tag, "x30 (t5):     %p", x[30]);
  logf(tag, "x31 (t6):     %p", x[31]);

  // unprivileged csrs

  uint64_t fflags;
  uint64_t frm;
  uint64_t fcsr;
  uint64_t cycle;
  uint64_t time;
  uint64_t instret;
  uint64_t hpmcounter[28];

  asm volatile("csrr %0, fflags" : "=r"(fflags));
  asm volatile("csrr %0, frm" : "=r"(frm));
  asm volatile("csrr %0, fcsr" : "=r"(fcsr));
  asm volatile("rdcycle %0" : "=r"(cycle));
  asm volatile("rdtime %0" : "=r"(time));
  asm volatile("rdinstret %0" : "=r"(instret));
  asm volatile("csrr %0, hpmcounter3" : "=r"(hpmcounter[0]));
  asm volatile("csrr %0, hpmcounter4" : "=r"(hpmcounter[1]));
  asm volatile("csrr %0, hpmcounter5" : "=r"(hpmcounter[2]));
  asm volatile("csrr %0, hpmcounter6" : "=r"(hpmcounter[3]));
  asm volatile("csrr %0, hpmcounter7" : "=r"(hpmcounter[4]));
  asm volatile("csrr %0, hpmcounter8" : "=r"(hpmcounter[5]));
  asm volatile("csrr %0, hpmcounter9" : "=r"(hpmcounter[6]));
  asm volatile("csrr %0, hpmcounter10" : "=r"(hpmcounter[7]));
  asm volatile("csrr %0, hpmcounter11" : "=r"(hpmcounter[8]));
  asm volatile("csrr %0, hpmcounter12" : "=r"(hpmcounter[9]));
  asm volatile("csrr %0, hpmcounter13" : "=r"(hpmcounter[10]));
  asm volatile("csrr %0, hpmcounter14" : "=r"(hpmcounter[11]));
  asm volatile("csrr %0, hpmcounter15" : "=r"(hpmcounter[12]));
  asm volatile("csrr %0, hpmcounter16" : "=r"(hpmcounter[13]));
  asm volatile("csrr %0, hpmcounter17" : "=r"(hpmcounter[14]));
  asm volatile("csrr %0, hpmcounter18" : "=r"(hpmcounter[15]));
  asm volatile("csrr %0, hpmcounter19" : "=r"(hpmcounter[16]));
  asm volatile("csrr %0, hpmcounter20" : "=r"(hpmcounter[17]));
  asm volatile("csrr %0, hpmcounter21" : "=r"(hpmcounter[18]));
  asm volatile("csrr %0, hpmcounter22" : "=r"(hpmcounter[19]));
  asm volatile("csrr %0, hpmcounter23" : "=r"(hpmcounter[20]));
  asm volatile("csrr %0, hpmcounter24" : "=r"(hpmcounter[21]));
  asm volatile("csrr %0, hpmcounter25" : "=r"(hpmcounter[22]));
  asm volatile("csrr %0, hpmcounter26" : "=r"(hpmcounter[23]));
  asm volatile("csrr %0, hpmcounter27" : "=r"(hpmcounter[24]));
  asm volatile("csrr %0, hpmcounter28" : "=r"(hpmcounter[25]));
  asm volatile("csrr %0, hpmcounter29" : "=r"(hpmcounter[26]));
  asm volatile("csrr %0, hpmcounter30" : "=r"(hpmcounter[27]));
  asm volatile("csrr %0, hpmcounter31" : "=r"(hpmcounter[28]));

  logf(tag, "fflags:       %p", fflags);
  logf(tag, "frm:          %p", frm);
  logf(tag, "fcsr:         %p", fcsr);
  logf(tag, "cycle:        %p", cycle);
  logf(tag, "time:         %p", time);
  logf(tag, "instret:      %p", instret);
  logf(tag, "hpmcounter3:  %p", hpmcounter[0]);
  logf(tag, "hpmcounter4:  %p", hpmcounter[1]);
  logf(tag, "hpmcounter5:  %p", hpmcounter[2]);
  logf(tag, "hpmcounter6:  %p", hpmcounter[3]);
  logf(tag, "hpmcounter7:  %p", hpmcounter[4]);
  logf(tag, "hpmcounter8:  %p", hpmcounter[5]);
  logf(tag, "hpmcounter9:  %p", hpmcounter[6]);
  logf(tag, "hpmcounter10: %p", hpmcounter[7]);
  logf(tag, "hpmcounter11: %p", hpmcounter[8]);
  logf(tag, "hpmcounter12: %p", hpmcounter[9]);
  logf(tag, "hpmcounter13: %p", hpmcounter[10]);
  logf(tag, "hpmcounter14: %p", hpmcounter[11]);
  logf(tag, "hpmcounter15: %p", hpmcounter[12]);
  logf(tag, "hpmcounter16: %p", hpmcounter[13]);
  logf(tag, "hpmcounter17: %p", hpmcounter[14]);
  logf(tag, "hpmcounter18: %p", hpmcounter[15]);
  logf(tag, "hpmcounter19: %p", hpmcounter[16]);
  logf(tag, "hpmcounter20: %p", hpmcounter[17]);
  logf(tag, "hpmcounter21: %p", hpmcounter[18]);
  logf(tag, "hpmcounter22: %p", hpmcounter[19]);
  logf(tag, "hpmcounter23: %p", hpmcounter[20]);
  logf(tag, "hpmcounter24: %p", hpmcounter[21]);
  logf(tag, "hpmcounter25: %p", hpmcounter[22]);
  logf(tag, "hpmcounter26: %p", hpmcounter[23]);
  logf(tag, "hpmcounter27: %p", hpmcounter[24]);
  logf(tag, "hpmcounter28: %p", hpmcounter[25]);
  logf(tag, "hpmcounter29: %p", hpmcounter[26]);
  logf(tag, "hpmcounter30: %p", hpmcounter[27]);
  logf(tag, "hpmcounter31: %p", hpmcounter[28]);

  // supervisor csrs

  uint64_t sstatus;
  uint64_t sie;
  uint64_t stvec;
  uint64_t scounteren;
  // uint64_t senvcfg;
  uint64_t sscratch;
  uint64_t sepc;
  uint64_t scause;
  uint64_t stval;
  uint64_t sip;
  uint64_t satp;
  // uint64_t scontext;

  asm volatile("csrr %0, sstatus" : "=r"(sstatus));
  asm volatile("csrr %0, sie" : "=r"(sie));
  asm volatile("csrr %0, stvec" : "=r"(stvec));
  asm volatile("csrr %0, scounteren" : "=r"(scounteren));
  asm volatile("csrr %0, sscratch" : "=r"(sscratch));
  asm volatile("csrr %0, sepc" : "=r"(sepc));
  asm volatile("csrr %0, scause" : "=r"(scause));
  asm volatile("csrr %0, stval" : "=r"(stval));
  asm volatile("csrr %0, sip" : "=r"(sip));
  asm volatile("csrr %0, satp" : "=r"(satp));

  logf(tag, "sstatus:      %p", sstatus);
  logf(tag, "sie:          %p", sie);
  logf(tag, "stvec:        %p", stvec);
  logf(tag, "scounteren:   %p", scounteren);
  logf(tag, "sscratch:     %p", sscratch);
  logf(tag, "sepc:         %p", sepc);
  logf(tag, "scause:       %p", scause);
  logf(tag, "stval:        %p", stval);
  logf(tag, "sip:          %p", sip);
}
