#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to read and print the content of CR0 register");
MODULE_VERSION("1.0");

static int __init cr0_module_init(void) {
    unsigned long cr0;

    // Read CR0
    __asm__ __volatile__(
        "mov %%cr0, %0\n"
        : "=r" (cr0)
    );

    printk(KERN_INFO "CR0 Module: CR0 value = 0x%lx\n", cr0);
    return 0;
}

static void __exit cr0_module_exit(void) {
    printk(KERN_INFO "CR0 Module: Unloading...\n");
}

module_init(cr0_module_init);
module_exit(cr0_module_exit);

