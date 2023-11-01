#ifndef KERNEL_ATTRIBUTE_H_
#define KERNEL_ATTRIBUTE_H_

#define __init_code __attribute__((section(".init.text")))
#define __init_data __attribute__((section(".init.data")))

#endif // KERNEL_ATTRIBUTE_H_
