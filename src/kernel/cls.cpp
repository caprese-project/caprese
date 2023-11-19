#include <kernel/cls.h>
#include <kernel/core_id.h>

namespace {
  core_local_storage_t core_local_storages[CONFIG_MAX_CORES];
} // namespace

map_ptr<core_local_storage_t> get_cls() {
  core_id_t core_id = get_core_id();
  return make_map_ptr(&core_local_storages[core_id]);
}
