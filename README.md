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
cmake -B build -G Ninja -DPLATFORM=<YOUR_PLATFORM>
```

3. Build
```sh
cmake --build build
```

If the target is qemu-riscv-virt, you can run the kernel with the following command.
```sh
./simulate.cmake
```

## Lisence

This project is released under the MIT License, see the [LICENSE](./LICENSE) file for details.
