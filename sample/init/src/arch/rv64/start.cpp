#include <bit>
#include <cstddef>
#include <cstdint>

#include <caprese/util/align.h>

#include <init/arch/boot.h>
#include <init/main.h>
#include <lib/debug.h>
#include <lib/syscall.h>

void run_mm([[maybe_unused]] init::boot_t* boot, cap_handle_t task_cap) {
  printd("run_mm\n");

  auto [list_size, list_size_error] = sys_cap_list_size();
  if (list_size_error) [[unlikely]] {
    return;
  }

  for (cap_handle_t handle = 0; handle < list_size; ++handle) {
    if (get_cap_type(handle) != CAP_TYPE_MEMORY) [[unlikely]] {
      continue;
    }

    auto [virt_addr, virt_addr_error] = sys_cap_get_field(handle, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS);
    if (virt_addr || virt_addr_error) [[unlikely]] {
      continue;
    }

    sys_cap_move(task_cap, handle);
  }

  sys_cap_call_method(task_cap, TASK_CAP_METHOD_SWITCH);
}

void run_plic(init::boot_t* boot, cap_handle_t task_cap) {
  printd("run_plic\n");
  // TODO: impl
  (void)boot;
  (void)task_cap;
}

void run_fs(init::boot_t* boot, cap_handle_t task_cap) {
  printd("run_fs\n");
  // TODO: impl
  (void)boot;
  (void)task_cap;
}

extern "C" {
  extern const char _mm_start[];
  extern const char _mm_end[];
  extern const char _plic_start[];
  extern const char _plic_end[];
  extern const char _fs_start[];
  extern const char _fs_end[];

  struct fdt_header_t {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
  };

  uint32_t __bswapsi2(uint32_t value) {
    return ((value & 0xff000000) >> 24) | ((value & 0x00ff0000) >> 8) | ((value & 0x0000ff00) << 8) | ((value & 0x000000ff) << 24);
  }

  void start(uintptr_t hartid, const char* dtb, cap_handle_t task_cap) {
    printd("hartid: 0x%lx\n", hartid);
    printd("dtb: 0x%lx\n", dtb);

    const fdt_header_t* fdt_header = reinterpret_cast<const fdt_header_t*>(dtb);
    const char*         dtb_end    = dtb + std::byteswap(fdt_header->totalsize);

    init::boot_t boot;
    boot.hartid             = hartid;
    boot.dtb                = dtb;
    boot.init_task_cap      = task_cap;
    boot.free_address_start = caprese::round_up(reinterpret_cast<uintptr_t>(dtb_end), 0x1000);
    boot.tasks[0].start     = _mm_start;
    boot.tasks[0].end       = _mm_end;
    boot.tasks[0].run       = run_mm;
    boot.tasks[1].start     = _plic_start;
    boot.tasks[1].end       = _plic_end;
    boot.tasks[1].run       = run_plic;
    boot.tasks[2].start     = _fs_start;
    boot.tasks[2].end       = _fs_end;
    boot.tasks[2].run       = run_fs;

    init::main(&boot);
  }
}
