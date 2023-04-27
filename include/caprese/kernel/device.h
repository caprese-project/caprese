#ifndef CAPRESE_KERNEL_DEVICE_H_
#define CAPRESE_KERNEL_DEVICE_H_

#include <cstddef>

#include <caprese/lib/error.h>

namespace caprese {
  struct device_node_property {
    const char* name;
    const char* data;
    size_t      data_length;
  };

  class device_node {
    friend class device;

    const char* begin_node_address;
    const char* begin_string_block;

  public:
    error_t find_node_by_name(device_node& node, const char* name) const;
    error_t find_property_by_name(device_node_property& property, const char* name) const;
  };

  class device {
    const char* dtb;

  public:
    error_t get_root_node(device_node& node) const;

  public:
    static error_t from_dtb(device& device, const char* dtb);
  };
} // namespace caprese

#endif // CAPRESE_KERNEL_DEVICE_H_
