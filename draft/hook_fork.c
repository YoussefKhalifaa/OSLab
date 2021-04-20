#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
//test
#define unprotect_memory(orig_cr0)\
({\
		orig_cr0 = read_cr0();\
		write_cr0(orig_cr0 & (~ 0x10000));\
});

#define protect_memory(orig_cr0)\
({		write_cr0(orig_cr0);});

asmlinkage long (*orig_shutdown)(int, int);
unsigned long *call_table;
unsigned int orig_cr0 = 0;

void hooking_syscall(void *hook_addr, uint16_t syscall_offset, unsigned long *call_table)
{
	unprotect_memory(orig_cr0);
	call_table[syscall_offset] = (unsigned long)hook_addr;
	protect_memory(orig_cr0);

}

void unhooking_syscall(void *orig_addr, uint16_t syscall_offset, unsigned long *call_table)
{
	unprotect_memory(orig_cr0);
	call_table[syscall_offset] = (unsigned long)orig_addr;
	protect_memory(orig_cr0);
}
asmlinkage int hooked_shutdown(int magic1, int magic2)
{
	printk("hello from hook!");
	return orig_shutdown(magic1, magic2);
}

int module_init1(void)
{
//	long unsigned int base_address = simple_strtoull("ffffffff81c00282", NULL, 16);
//	printk("Address --> %#lx\n", base_address);
	
	long unsigned int table_address = 0xffffffff81c00282;
//	printk("address --> %lx\n", table_address);
//	printk("Address --> %lx\n", kallsyms_lookup_name("sys_call_table"));
	call_table = table_address; 

//	call_table = (void *)kallsyms_lookup_name("sys_call_table");

	orig_shutdown = (void *)call_table[__NR_chdir];
	hooking_syscall(hooked_shutdown, __NR_chdir, call_table);

	return 0;
}

void module_cleanup1(void)
{
	printk("bye...\n");
	unhooking_syscall(orig_shutdown, __NR_chdir, call_table);
}

module_init(module_init1);
module_exit(module_cleanup1);
MODULE_LICENSE("GPL");
