#ifndef ARCH_RV64_KERNEL_SBI_H_
#define ARCH_RV64_KERNEL_SBI_H_

#include <cstdint>

struct sbiret_t {
  long error;
  long value;
};

inline sbiret_t sbicall0(unsigned long eid, unsigned long fid) {
  sbiret_t ret;

  asm volatile("mv a7, %0" : : "r"(eid));
  asm volatile("mv a6, %0" : : "r"(fid));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(ret.error));
  asm volatile("mv %0, a1" : "=r"(ret.value));

  return ret;
}

inline sbiret_t sbicall1(unsigned long eid, unsigned long fid, unsigned long arg0) {
  sbiret_t ret;

  asm volatile("mv a7, %0" : : "r"(eid));
  asm volatile("mv a6, %0" : : "r"(fid));
  asm volatile("mv a0, %0" : : "r"(arg0));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(ret.error));
  asm volatile("mv %0, a1" : "=r"(ret.value));

  return ret;
}

inline sbiret_t sbicall2(unsigned long eid, unsigned long fid, unsigned long arg0, unsigned long arg1) {
  sbiret_t ret;

  asm volatile("mv a7, %0" : : "r"(eid));
  asm volatile("mv a6, %0" : : "r"(fid));
  asm volatile("mv a0, %0" : : "r"(arg0));
  asm volatile("mv a1, %0" : : "r"(arg1));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(ret.error));
  asm volatile("mv %0, a1" : "=r"(ret.value));

  return ret;
}

inline sbiret_t sbicall3(unsigned long eid, unsigned long fid, unsigned long arg0, unsigned long arg1, unsigned long arg2) {
  sbiret_t ret;

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

inline sbiret_t sbicall4(unsigned long eid, unsigned long fid, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3) {
  sbiret_t ret;

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

inline sbiret_t sbicall5(unsigned long eid, unsigned long fid, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4) {
  sbiret_t ret;

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

inline sbiret_t sbicall6(unsigned long eid, unsigned long fid, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) {
  sbiret_t ret;

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

inline long sbi_console_putchar(int ch) {
  return sbicall1(1, 0, ch).error;
}

inline long sbi_console_getchar() {
  return sbicall0(2, 0).error;
}

inline long sbi_clear_ipi() {
  return sbicall0(3, 0).error;
}

inline void sbi_shutdown() {
  sbicall0(8, 0);
}

inline sbiret_t sbi_get_spec_version() {
  return sbicall0(0x10, 0);
}

inline sbiret_t sbi_get_impl_id() {
  return sbicall0(0x10, 1);
}

inline sbiret_t sbi_get_impl_version() {
  return sbicall0(0x10, 2);
}

inline sbiret_t sbi_probe_extension(long extension_id) {
  return sbicall1(0x10, 3, extension_id);
}

inline sbiret_t sbi_get_mvendorid() {
  return sbicall0(0x10, 4);
}

inline sbiret_t sbi_get_marchid() {
  return sbicall0(0x10, 5);
}

inline sbiret_t sbi_get_mimpid() {
  return sbicall0(0x10, 6);
}

inline sbiret_t sbi_set_timer(unsigned long stime_value) {
  return sbicall1(0x54494D45, 0, stime_value);
}

inline sbiret_t sbi_send_ipi(unsigned long hart_mask, unsigned long hart_mask_base) {
  return sbicall2(0x735049, 0, hart_mask, hart_mask_base);
}

inline sbiret_t sbi_remote_fence_i(unsigned long hart_mask, unsigned long hart_mask_base) {
  return sbicall2(0x52464E43, 0, hart_mask, hart_mask_base);
}

inline sbiret_t sbi_remote_sfence_vma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
  return sbicall4(0x52464E43, 1, hart_mask, hart_mask_base, start_addr, size);
}

inline sbiret_t sbi_remote_sfence_vma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size, unsigned long asid) {
  return sbicall5(0x52464E43, 2, hart_mask, hart_mask_base, start_addr, size, asid);
}

inline sbiret_t sbi_remote_hfence_gvma_vmid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size, unsigned long vmid) {
  return sbicall5(0x52464E43, 3, hart_mask, hart_mask_base, start_addr, size, vmid);
}

inline sbiret_t sbi_remote_hfence_gvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
  return sbicall4(0x52464E43, 4, hart_mask, hart_mask_base, start_addr, size);
}

inline sbiret_t sbi_remote_hfence_vvma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size, unsigned long asid) {
  return sbicall5(0x52464E43, 5, hart_mask, hart_mask_base, start_addr, size, asid);
}

inline sbiret_t sbi_remote_hfence_vvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
  return sbicall4(0x52464E43, 6, hart_mask, hart_mask_base, start_addr, size);
}

inline sbiret_t sbi_hart_start(unsigned long hartid, unsigned long start_addr, unsigned long opaque) {
  return sbicall3(0x48534D, 0, hartid, start_addr, opaque);
}

inline sbiret_t sbi_hart_stop() {
  return sbicall0(0x48534D, 1);
}

inline sbiret_t sbi_hart_get_status(unsigned long hartid) {
  return sbicall1(0x48534D, 2, hartid);
}

inline sbiret_t sbi_hart_suspend(uint32_t suspend_type, unsigned long resume_addr, unsigned long opaque) {
  return sbicall3(0x48534D, 3, suspend_type, resume_addr, opaque);
}

inline sbiret_t sbi_system_reset(uint32_t reset_type, uint32_t reset_reason) {
  return sbicall2(0x53525354, 0, reset_type, reset_reason);
}

inline sbiret_t sbi_pmu_num_counters() {
  return sbicall0(0x504D55, 0);
}

inline sbiret_t sbi_pmu_counter_get_info(unsigned long counter_idx) {
  return sbicall1(0x504D55, 1, counter_idx);
}

inline sbiret_t sbi_pmu_counter_config_matching(unsigned long counter_idx_base, unsigned long counter_idx_mask, unsigned long config_flags, unsigned long event_idx, uint64_t event_data) {
  return sbicall5(0x504D55, 2, counter_idx_base, counter_idx_mask, config_flags, event_idx, event_data);
}

inline sbiret_t sbi_pmu_counter_start(unsigned long counter_idx_base, unsigned long counter_idx_mask, unsigned long start_flags, uint64_t initial_value) {
  return sbicall4(0x504D55, 3, counter_idx_base, counter_idx_mask, start_flags, initial_value);
}

inline sbiret_t sbi_pmu_counter_stop(unsigned long counter_idx_base, unsigned long counter_idx_mask, unsigned long stop_flags) {
  return sbicall3(0x504D55, 4, counter_idx_base, counter_idx_mask, stop_flags);
}

inline sbiret_t sbi_pmu_counter_fw_read(unsigned long counter_idx) {
  return sbicall1(0x504D55, 5, counter_idx);
}

inline sbiret_t sbi_pmu_counter_fw_read_hi(unsigned long counter_idx) {
  return sbicall1(0x504D55, 6, counter_idx);
}

inline sbiret_t sbi_debug_console_write(unsigned long num_bytes, unsigned long base_addr_lo, unsigned long base_addr_hi) {
  return sbicall3(0x4442434E, 0, num_bytes, base_addr_lo, base_addr_hi);
}

inline sbiret_t sbi_debug_console_read(unsigned long num_bytes, unsigned long base_addr_lo, unsigned long base_addr_hi) {
  return sbicall3(0x4442434E, 1, num_bytes, base_addr_lo, base_addr_hi);
}

inline sbiret_t sbi_debug_console_write_byte(uint8_t byte) {
  return sbicall1(0x4442434E, 2, byte);
}

inline sbiret_t sbi_system_suspend(uint32_t sleep_type, unsigned long resume_addr, unsigned long opaque) {
  return sbicall3(0x53555350, 0, sleep_type, resume_addr, opaque);
}

inline sbiret_t sbi_cppc_probe(uint32_t cppc_reg_id) {
  return sbicall1(0x43505043, 0, cppc_reg_id);
}

inline sbiret_t sbi_cppc_read(uint32_t cppc_reg_id) {
  return sbicall1(0x43505043, 1, cppc_reg_id);
}

inline sbiret_t sbi_cppc_read_hi(uint32_t cppc_reg_id) {
  return sbicall1(0x43505043, 2, cppc_reg_id);
}

inline sbiret_t sbi_cppc_write(uint32_t cppc_reg_id, uint64_t val) {
  return sbicall2(0x43505043, 3, cppc_reg_id, val);
}

#endif // ARCH_RV64_KERNEL_SBI_H_
