#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>

MODULE_LICENSE("GPL");

void    **sys_call_table_ptr;
int counter = 0;
asmlinkage int (*old_fork)(void);

asmlinkage int new_fork(void)
{
//    printk("Khalifa hooked\n");
    counter ++;
    if(counter%10==0)
    {
	printk("Fork counter = %d\n", counter);
    }
    
    return old_fork();
}

static int __init myfork_init(void)
{
//	printk("Helloo...\n");/*
	//table address from previous exercise
	unsigned long table_address = simple_strtoull("ffffffff81c00282", NULL, 16);
	
	write_cr0(read_cr0() & (~0x10000)); //modifying cr0 register
    	sys_call_table_ptr = (void**)table_address; //setting pointer to table address
    	old_fork = sys_call_table_ptr[__NR_execve]; //saving original system call
    	sys_call_table_ptr[__NR_execve] = new_fork; //hooking my system call
    	write_cr0(read_cr0() | (0x10000)); //restoring cr0 register
//  
	return 0;
}

static void __exit myfork_exit(void)
{
//	printk("Bye...\n");/*
        write_cr0(read_cr0() & (~0x10000)); //modifying cr0 register
    	sys_call_table_ptr[__NR_execve] = old_fork; //restoring original system call
    	write_cr0(read_cr0() | (0x10000)); //restoring cr0 register	
//
}

module_init(myfork_init);
module_exit(myfork_exit);
