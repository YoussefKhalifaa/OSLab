#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>

#define unprotect_memory()(\
		orig_cr0 = read_cr0();\
		write_cr0(orig_cr0 & (~ 0x10000));\
		);

#define protect_memory()(\
		write_cr0(orig_cr0);\
		);

asmlinkage long (*orig_shutdown)(int, int);
unsigned long *call_table;

void hooking_syscall(void *hook_addr, uint16_t syscall_offset, unsigned long *call_table)
{
	unprotect_memory();
	call_table[syscall_offset] = (unsigned long)hook_addr;
	protect_memory();

}

void unhooking_suscall(void *orig_addr, uint16_t syscall_offset)
{
	unprotect_memory();
	call_table[syscall_offset] = (unsigned long)orig_addr;
	protect_memory();

asmlinkage int hooked_shutdown(int magic1, int magic2)
{
	printk("hello from hook!");
	return orig_shutdown(magic1, magic2);
}

static int __init module_init(void)
{
	unsigned long *call_table = kallsyms_lookup_name("sys_call_table");
	orig_shutdown = (void*)call_table[__NR_shutdown];
	hooking_syscall(hooked_shutdown, __NR_shutdown, call_table);

	return 0;
}

static void __exit module_cleanup(void)
{
	unhooking_syscall(orig_shutdown, __NR_shutdown, call_table);
}

module_init(module_init);
module_exit(module_cleanup);
//MODULE_LICENSE("GPL");
