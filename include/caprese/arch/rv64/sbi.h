/**
 * @file sbi.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief SBI wrappers.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#ifndef CAPRESE_ARCH_RV64_SBI_H_
#define CAPRESE_ARCH_RV64_SBI_H_

#if defined(CONFIG_ARCH_RISCV) && defined(CONFIG_XLEN_64)

#include <cstdint>

namespace caprese::arch::inline rv64 {
  inline namespace sbi {
    struct sbiret {
      long error;
      long value;
    };

    inline sbiret sbicall0(unsigned long eid, unsigned long fid) {
      sbiret ret;

      asm volatile("mv a7, %0" : : "r"(eid));
      asm volatile("mv a6, %0" : : "r"(fid));
      asm volatile("ecall");
      asm volatile("mv %0, a0" : "=r"(ret.error));
      asm volatile("mv %0, a1" : "=r"(ret.value));

      return ret;
    }

    template<typename T>
    inline sbiret sbicall1(unsigned long eid, unsigned long fid, T arg0) {
      sbiret ret;

      asm volatile("mv a7, %0" : : "r"(eid));
      asm volatile("mv a6, %0" : : "r"(fid));
      asm volatile("mv a0, %0" : : "r"(arg0));
      asm volatile("ecall");
      asm volatile("mv %0, a0" : "=r"(ret.error));
      asm volatile("mv %0, a1" : "=r"(ret.value));

      return ret;
    }

    template<typename T, typename U>
    inline sbiret sbicall2(unsigned long eid, unsigned long fid, T arg0, U arg1) {
      sbiret ret;

      asm volatile("mv a7, %0" : : "r"(eid));
      asm volatile("mv a6, %0" : : "r"(fid));
      asm volatile("mv a0, %0" : : "r"(arg0));
      asm volatile("mv a1, %0" : : "r"(arg1));
      asm volatile("ecall");
      asm volatile("mv %0, a0" : "=r"(ret.error));
      asm volatile("mv %0, a1" : "=r"(ret.value));

      return ret;
    }

    template<typename T, typename U, typename V>
    inline sbiret sbicall3(unsigned long eid, unsigned long fid, T arg0, U arg1, V arg2) {
      sbiret ret;

      asm volatile("mv a7, %0" : : "r"(eid));
      asm volatile("mv a6, %0" : : "r"(fid));
      asm volatile("mv a0, %0" : : "r"(arg0));
      asm volatile("mv a1, %0" : : "r"(arg1));
      asm volatile("mv a2, %0" : : "r"(arg2));
      asm volatile("ecall");
      asm volatile("mv %0, a0" : "=r"(ret.error));
      asm volatile("mv %0, a1" : "=r"(ret.value));

      return ret;
    }

    template<typename T, typename U, typename V, typename W>
    inline sbiret sbicall4(unsigned long eid, unsigned long fid, T arg0, U arg1, V arg2, W arg3) {
      sbiret ret;

      asm volatile("mv a7, %0" : : "r"(eid));
      asm volatile("mv a6, %0" : : "r"(fid));
      asm volatile("mv a0, %0" : : "r"(arg0));
      asm volatile("mv a1, %0" : : "r"(arg1));
      asm volatile("mv a2, %0" : : "r"(arg2));
      asm volatile("mv a3, %0" : : "r"(arg3));
      asm volatile("ecall");
      asm volatile("mv %0, a0" : "=r"(ret.error));
      asm volatile("mv %0, a1" : "=r"(ret.value));

      return ret;
    }

    template<typename T, typename U, typename V, typename W, typename X>
    inline sbiret sbicall5(unsigned long eid, unsigned long fid, T arg0, U arg1, V arg2, W arg3, X arg4) {
      sbiret ret;

      asm volatile("mv a7, %0" : : "r"(eid));
      asm volatile("mv a6, %0" : : "r"(fid));
      asm volatile("mv a0, %0" : : "r"(arg0));
      asm volatile("mv a1, %0" : : "r"(arg1));
      asm volatile("mv a2, %0" : : "r"(arg2));
      asm volatile("mv a3, %0" : : "r"(arg3));
      asm volatile("mv a4, %0" : : "r"(arg4));
      asm volatile("ecall");
      asm volatile("mv %0, a0" : "=r"(ret.error));
      asm volatile("mv %0, a1" : "=r"(ret.value));

      return ret;
    }

    template<typename T, typename U, typename V, typename W, typename X, typename Y>
    inline sbiret sbicall6(unsigned long eid, unsigned long fid, T arg0, U arg1, V arg2, W arg3, X arg4, Y arg5) {
      sbiret ret;

      asm volatile("mv a7, %0" : : "r"(eid));
      asm volatile("mv a6, %0" : : "r"(fid));
      asm volatile("mv a0, %0" : : "r"(arg0));
      asm volatile("mv a1, %0" : : "r"(arg1));
      asm volatile("mv a2, %0" : : "r"(arg2));
      asm volatile("mv a3, %0" : : "r"(arg3));
      asm volatile("mv a4, %0" : : "r"(arg4));
      asm volatile("mv a5, %0" : : "r"(arg5));
      asm volatile("ecall");
      asm volatile("mv %0, a0" : "=r"(ret.error));
      asm volatile("mv %0, a1" : "=r"(ret.value));

      return ret;
    }

    inline namespace legacy_extension {
      [[deprecated]] inline long sbi_set_timer(uint64_t stime_value) {
        return sbicall1(0, 0, stime_value).error;
      }

      inline long sbi_console_putchar(int ch) {
        return sbicall1(1, 0, ch).error;
      }

      inline long sbi_console_getchar() {
        return sbicall0(2, 0).error;
      }

      inline long sbi_clear_ipi() {
        return sbicall0(3, 0).error;
      }

      [[deprecated]] inline long sbi_send_ipi(const unsigned long* hart_mask) {
        return sbicall1(4, 0, hart_mask).error;
      }

      [[deprecated]] inline long sbi_remote_fence_i(const unsigned long* hart_mask) {
        return sbicall1(5, 0, hart_mask).error;
      }

      [[deprecated]] inline long sbi_remote_sfence_vma(const unsigned long* hart_mask, unsigned long start, unsigned long size) {
        return sbicall3(6, 0, hart_mask, start, size).error;
      }

      [[deprecated]] inline long long sbi_remote_sfence_vma_asid(const unsigned long* hart_mask, unsigned long start, unsigned long size, unsigned long asid) {
        return sbicall4(7, 0, hart_mask, start, size, asid).error;
      }

      [[deprecated]] inline void sbi_shutdown() {
        sbicall0(8, 0);
      }
    } // namespace legacy_extension

    inline namespace base_extension {
      constexpr unsigned long EID = 0x10;

      inline sbiret sbi_get_spec_version() {
        return sbicall0(EID, 0);
      }

      inline sbiret sbi_get_impl_id() {
        return sbicall0(EID, 1);
      }

      inline sbiret sbi_get_impl_version() {
        return sbicall0(EID, 2);
      }

      inline sbiret sbi_probe_extension(long extension_id) {
        return sbicall1(EID, 3, extension_id);
      }

      inline sbiret sbi_get_mvendorid() {
        return sbicall0(EID, 4);
      }

      inline sbiret sbi_get_marchid() {
        return sbicall0(EID, 5);
      }

      inline sbiret sbi_get_mimpid() {
        return sbicall0(EID, 6);
      }
    } // namespace base_extension

    inline namespace timer_extension {
      constexpr unsigned long EID = 0x54494D45;

      inline sbiret sbi_set_timer(unsigned long stime_value) {
        return sbicall1(EID, 0, stime_value);
      }
    } // namespace timer_extension

    inline namespace ipi_extension {
      constexpr unsigned long EID = 0x735049;

      inline sbiret sbi_send_ipi(unsigned long hart_mask, unsigned long hart_mask_base) {
        return sbicall2(EID, 0, hart_mask, hart_mask_base);
      }
    } // namespace ipi_extension

    inline namespace rfence_extension {
      constexpr unsigned long EID = 0x52464E43;

      inline sbiret sbi_remote_fence_i(unsigned long hart_mask, unsigned long hart_mask_base) {
        return sbicall2(EID, 0, hart_mask, hart_mask_base);
      }

      inline sbiret sbi_remote_sfence_vma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
        return sbicall4(EID, 1, hart_mask, hart_mask_base, start_addr, size);
      }

      inline sbiret
          sbi_remote_sfence_vma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size, unsigned long asid) {
        return sbicall5(EID, 2, hart_mask, hart_mask_base, start_addr, size, asid);
      }

      inline sbiret
          sbi_remote_hfence_gvma_vmid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size, unsigned long vmid) {
        return sbicall5(EID, 3, hart_mask, hart_mask_base, start_addr, size, vmid);
      }

      inline sbiret sbi_remote_hfence_gvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
        return sbicall4(EID, 4, hart_mask, hart_mask_base, start_addr, size);
      }

      inline sbiret
          sbi_remote_hfence_vvma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size, unsigned long asid) {
        return sbicall5(EID, 5, hart_mask, hart_mask_base, start_addr, size, asid);
      }

      inline sbiret sbi_remote_hfence_vvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
        return sbicall4(EID, 6, hart_mask, hart_mask_base, start_addr, size);
      }
    } // namespace rfence_extension

    inline namespace hart_state_management_extension {
      constexpr unsigned long EID = 0x48534D;

      inline sbiret sbi_hart_start(unsigned long hartid, unsigned long start_addr, unsigned long opaque) {
        return sbicall3(EID, 0, hartid, start_addr, opaque);
      }

      inline sbiret sbi_hart_stop() {
        return sbicall0(EID, 1);
      }

      inline sbiret sbi_hart_get_status(unsigned long hartid) {
        return sbicall1(EID, 2, hartid);
      }

      inline sbiret sbi_hart_suspend(uint32_t suspend_type, unsigned long resume_addr, unsigned long opaque) {
        return sbicall3(EID, 3, suspend_type, resume_addr, opaque);
      }
    } // namespace hart_state_management_extension

    inline namespace system_reset_extension {
      constexpr unsigned long EID = 0x53525354;

      inline sbiret sbi_system_reset(uint32_t reset_type, uint32_t reset_reason) {
        return sbicall2(EID, 0, reset_type, reset_reason);
      }
    } // namespace system_reset_extension

    inline namespace performance_monitoring_unit_extension {
      constexpr unsigned long EID = 0x504D55;

      inline sbiret sbi_pmu_num_counters() {
        return sbicall0(EID, 0);
      }

      inline sbiret sbi_pmu_counter_get_info(unsigned long counter_idx) {
        return sbicall1(EID, 1, counter_idx);
      }

      inline sbiret sbi_pmu_counter_config_matching(
          unsigned long counter_idx_base, unsigned long counter_idx_mask, unsigned long config_flags, unsigned long event_idx, uint64_t event_data) {
        return sbicall5(EID, 2, counter_idx_base, counter_idx_mask, config_flags, event_idx, event_data);
      }

      inline sbiret sbi_pmu_counter_start(unsigned long counter_idx_base, unsigned long counter_idx_mask, unsigned long start_flags, uint64_t initial_value) {
        return sbicall4(EID, 3, counter_idx_base, counter_idx_mask, start_flags, initial_value);
      }

      inline sbiret sbi_pmu_counter_stop(unsigned long counter_idx_base, unsigned long counter_idx_mask, unsigned long stop_flags) {
        return sbicall3(EID, 4, counter_idx_base, counter_idx_mask, stop_flags);
      }

      inline sbiret sbi_pmu_counter_fw_read(unsigned long counter_idx) {
        return sbicall1(EID, 5, counter_idx);
      }

      inline sbiret sbi_pmu_counter_fw_read_hi(unsigned long counter_idx) {
        return sbicall1(EID, 6, counter_idx);
      }
    } // namespace performance_monitoring_unit_extension

    inline namespace debug_console_extension {
      constexpr unsigned long EID = 0x4442434E;

      inline sbiret sbi_debug_console_write(unsigned long num_bytes, unsigned long base_addr_lo, unsigned long base_addr_hi) {
        return sbicall3(EID, 0, num_bytes, base_addr_lo, base_addr_hi);
      }

      inline sbiret sbi_debug_console_read(unsigned long num_bytes, unsigned long base_addr_lo, unsigned long base_addr_hi) {
        return sbicall3(EID, 1, num_bytes, base_addr_lo, base_addr_hi);
      }

      inline sbiret sbi_debug_console_write_byte(uint8_t byte) {
        return sbicall1(EID, 2, byte);
      }
    } // namespace debug_console_extension

    inline namespace sbi_system_suspend_extension {
      constexpr unsigned long EID = 0x53555350;

      inline sbiret sbi_system_suspend(uint32_t sleep_type, unsigned long resume_addr, unsigned long opaque) {
        return sbicall3(EID, 0, sleep_type, resume_addr, opaque);
      }
    } // namespace sbi_system_suspend_extension

    inline namespace collaborative_processor_performance_control_extension {
      constexpr unsigned long EID = 0x43505043;

      inline sbiret sbi_cppc_probe(uint32_t cppc_reg_id) {
        return sbicall1(EID, 0, cppc_reg_id);
      }

      inline sbiret sbi_cppc_read(uint32_t cppc_reg_id) {
        return sbicall1(EID, 1, cppc_reg_id);
      }

      inline sbiret sbi_cppc_read_hi(uint32_t cppc_reg_id) {
        return sbicall1(EID, 2, cppc_reg_id);
      }

      inline sbiret sbi_cppc_write(uint32_t cppc_reg_id, uint64_t val) {
        return sbicall2(EID, 3, cppc_reg_id, val);
      }
    } // namespace collaborative_processor_performance_control_extension
  }   // namespace sbi

} // namespace caprese::arch

#endif // defined(CONFIG_ARCH_RISCV) && defined(CONFIG_XLEN_64)

#endif // CAPRESE_ARCH_RV64_SBI_H_
