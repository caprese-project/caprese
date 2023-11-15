#ifndef ARCH_RV64_KERNEL_CSR_H_
#define ARCH_RV64_KERNEL_CSR_H_

#include <cstdint>

/**
 * @brief sd field in sstatus register
 *
 * The SD bit is a read-only bit that summarizes whether either the FS, VS, or XS fields signal the presence of some dirty state that will require saving
 * extended user context to memory.
 * If FS, XS, and VS are all read-only zero, then SD is also always zero.
 */
constexpr uint64_t SSTATUS_SD   = 0b1ull << 63;
/**
 * @brief uxl field in sstatus register
 *
 * The UXL field controls the value of XLEN for U-mode, termed UXLEN, which may differ from the value of XLEN for S-mode, termed SXLEN.
 * When SXLEN=32, the UXL field does not exist, and UXLEN=32. When SXLEN=64, it is a WARL field that encodes the current value of UXLEN.
 * In particular, an implementation may make UXL be a read-only field whose value always ensures that UXLEN=SXLEN.
 */
constexpr uint64_t SSTATUS_UXL  = 0b11ull << 32;
/**
 * @brief mxr field in sstatus register
 *
 * The MXR (Make eXecutable Readable) bit modifies the privilege with which loads access virtual memory.
 * When MXR=0, only loads from pages marked readable (R=1 in Figure 4.18) will succeed.
 * When MXR=1, loads from pages marked either readable or executable (R=1 or X=1) will succeed.
 * MXR has no effect when page-based virtual memory is not in effect.
 */
constexpr uint64_t SSTATUS_MXR  = 0b1ull << 19;
/**
 * @brief sum field in sstatus register
 *
 * The SUM (permit Supervisor User Memory access) bit modifies the privilege with which S-mode loads and stores access virtual memory.
 * When SUM=0, S-mode memory accesses to pages that are accessible by U-mode (U=1 in Figure 4.18) will fault.
 * When SUM=1, these accesses are permitted.
 * SUM has no effect when page-based virtual memory is not in effect, nor when executing in U-mode.
 * Note that S-mode can never execute instructions from user pages, regardless of the state of SUM.
 */
constexpr uint64_t SSTATUS_SUM  = 0b1ull << 18;
/**
 * @brief xs field in sstatus register
 *
 * The XS field encodes the status of additional user-mode extensions and associated state.
 */
constexpr uint64_t SSTATUS_XS   = 0b11ull << 15;
/**
 * @brief fs field in sstatus register
 *
 * The FS field encodes the status of the floating-point unit state, including the floating-point registers f0-f31 and the CSRs fcsr, frm, and fflags.
 */
constexpr uint64_t SSTATUS_FS   = 0b11ull << 13;
/**
 * @brief vs field in sstatus register
 *
 * The VS field encodes the status of the vector extension state, including the vector registers v0-v31 and the CSRs vcsr, vxrm, vxsat, vstart, vl, vtype, and
 * vlenb.
 */
constexpr uint64_t SSTATUS_VS   = 0b11ull << 9;
/**
 * @brief spp field in sstatus register
 *
 * The SPP bit holds the previous privilege mode.
 * When a trap is taken from privilege mode y into privilege mode x, xPP is set to y.
 */
constexpr uint64_t SSTATUS_SPP  = 0b1ull << 8;
/**
 * @brief ube field in sstatus register
 *
 * The UBE bit is a WARL field that controls the endianness of explicit memory accesses made from U-mode, which may differ from the endianness of memory
 * accesses in S-mode.
 * An implementation may make UBE be a read-only field that always specifies the same endianness as for S-mode.
 * UBE controls whether explicit load and store memory accesses made from U-mode are little-endian (UBE=0) or big-endian (UBE=1).
 * UBE has no effect on instruction fetches, which are implicit memory accesses that are alwayslittle-endian.
 */
constexpr uint64_t SSTATUS_UBE  = 0b1ull << 6;
/**
 * @brief spie field in sstatus register
 *
 * The SPIE bit holds the value of the interrupt-enable bit active prior to the trap.
 * When a trap is taken from privilege mode y into privilege mode x, xPIE is set to the value of xIE and xIE is set to 0.
 */
constexpr uint64_t SSTATUS_SPIE = 0b1ull << 5;
/**
 * @brief sie field in sstatus register
 *
 * When a hart is executing in S-mode, interrupts are globally enabled when SIE=1 and globally disabled when SIE=0.
 */
constexpr uint64_t SSTATUS_SIE  = 0b1ull << 1;

constexpr uint64_t SSTATUS_FS_VS_XS_OFF     = 0b00;
constexpr uint64_t SSTATUS_FS_VS_XS_INITIAL = 0b01;
constexpr uint64_t SSTATUS_FS_VS_XS_CLEAN   = 0b10;
constexpr uint64_t SSTATUS_FS_VS_XS_DIRTY   = 0b11;

constexpr uint64_t STVEC_MODE = 0b11ull;
constexpr uint64_t STVEC_BASE = ~STVEC_MODE;

/**
 * @brief Flag that stvec mode is direct
 *
 * All exceptions set pc to STVEC_BASE.
 */
constexpr uint64_t STVEC_MODE_DIRECT = 0b00;
/**
 * @brief Flag that stvec mode is vector
 *
 * Asynchronous interrupts set pc to STVEC_BASE + 4 * cause.
 */
constexpr uint64_t STVEC_MODE_VECTOR = 0b01;

constexpr uint64_t SIP_SEIP = 0b1ull << 9;
constexpr uint64_t SIP_STIP = 0b1ull << 5;
constexpr uint64_t SIP_SSIP = 0b1ull << 1;

constexpr uint64_t SIE_SEIE = 0b1ull << 9;
constexpr uint64_t SIE_STIE = 0b1ull << 5;
constexpr uint64_t SIE_SSIE = 0b1ull << 1;

constexpr uint64_t SCAUSE_EXCEPTION_CODE = (0b1ull << 63) - 1;
constexpr uint64_t SCAUSE_INTERRUPT      = 0b1ull << 63;

constexpr uint64_t SCAUSE_SUPERVISOR_SOFTWARE_INTERRUPT = 0x1;
constexpr uint64_t SCAUSE_SUPERVISOR_TIMER_INTERRUPT    = 0x5;
constexpr uint64_t SCAUSE_SUPERVISOR_EXTERNAL_INTERRUPT = 0x9;

constexpr uint64_t SCAUSE_INSTRUCTION_ADDRESS_MISALIGNED = 0x0;
constexpr uint64_t SCAUSE_INSTRUCTION_ACCESS_FAULT       = 0x1;
constexpr uint64_t SCAUSE_ILLEGAL_INSTRUCTION            = 0x2;
constexpr uint64_t SCAUSE_BREAKPOINT                     = 0x3;
constexpr uint64_t SCAUSE_LOAD_ADDRESS_MISALIGNED        = 0x4;
constexpr uint64_t SCAUSE_LOAD_ACCESS_FAULT              = 0x5;
constexpr uint64_t SCAUSE_STORE_AMO_ADDRESS_MISALIGNED   = 0x6;
constexpr uint64_t SCAUSE_STORE_AMO_ACCESS_FAULT         = 0x7;
constexpr uint64_t SCAUSE_ENVIRONMENT_CALL_FROM_U_MODE   = 0x8;
constexpr uint64_t SCAUSE_ENVIRONMENT_CALL_FROM_S_MODE   = 0x9;
constexpr uint64_t SCAUSE_ENVIRONMENT_CALL_FROM_M_MODE   = 0xB;
constexpr uint64_t SCAUSE_INSTRUCTION_PAGE_FAULT         = 0xC;
constexpr uint64_t SCAUSE_LOAD_PAGE_FAULT                = 0xD;
constexpr uint64_t SCAUSE_STORE_AMO_PAGE_FAULT           = 0xF;

constexpr uint64_t SATP_PPN  = (1ull << 44) - 1;
constexpr uint64_t SATP_ASID = ((1ull << 60) - 1) & ~SATP_PPN;
constexpr uint64_t SATP_MODE = 0b1111ull << 60;

constexpr uint64_t SATP_MODE_BARE = 0b0000ull << 60;
constexpr uint64_t SATP_MODE_SV39 = 0b1000ull << 60;
constexpr uint64_t SATP_MODE_SV48 = 0b1001ull << 60;

#endif // ARCH_RV64_KERNEL_CSR_H_
