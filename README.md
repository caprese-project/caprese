# Caprese

## What is Caprese?

Caprese is a microkernel that protects resources with capability-based security.

## Fetching, Configuring and Building Caprese

1. Clone Caprese from GitHub
```sh
git clone https://github.com/cosocaf/caprese
cd caprese
```

2. Configure CMake
```sh
# <YOUR_PLATFORM> is the target platform, see cmake/platforms.
cmake -B build -G Ninja -DPLATFORM=<YOUR_PLATFORM> -DROOT=<YOUR_ROOT_TARGET_DIR>
```

3. Build
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
