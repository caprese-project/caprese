# Caprese

## What is Caprese?

Caprese is a microkernel that protects resources with capability-based security.

## Fetching, Configuring and Building

1. Install libcaprese

    See <https://github.com/cosocaf/libcaprese>

2. Clone Caprese from GitHub
```sh
git clone https://github.com/cosocaf/caprese
cd caprese
```

3. Configure CMake
```sh
# <YOUR_PLATFORM> is the target platform, see cmake/platforms.
cmake -B build -G Ninja -DPLATFORM=<YOUR_PLATFORM> -DCONFIG_ROOT_SERVER_BIN=<YOUR_ROOT_SERVER_BIN> -DCONFIG_ROOT_SERVER_BASE_ADDRESS=<0x00000000>
```

4. Build
```sh
cmake --build build
```

If the target is qemu-riscv-virt, you can run the kernel with the following command.
```sh
./scripts/simulate
```

## Lisence

This project is released under the MIT License, see the [LICENSE](./LICENSE) file for details.

## Acknowledgments

- IPA 未踏IT人材発掘・育成事業 (<https://www.ipa.go.jp/jinzai/mitou/it/2023/gaiyou_tn-4.html>)
