#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#define BUFSIZE 100
MODULE_LICENSE("Dual BSD/GPL");

static struct proc_dir_entry *ent;
//random fork counter value since code from previous exercise was buggy
static int fork_counter=20; 

static ssize_t reset_fork(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	int num, c, f;
	char buf[BUFSIZE];
	if(*ppos > 0 || count > BUFSIZE)
		return -EFAULT;
	if(copy_from_user(buf, ubuf, count))
		return -EFAULT;
	num = sscanf(buf, "%d", &f);
	if(num !=1)
		return -EFAULT;
	fork_counter = f;
	c = strlen(buf);
	*ppos = c;

	printk(KERN_DEBUG "resetting fork counter\n");
	return c;
}


static ssize_t read_fork(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[BUFSIZE];
	int len = 0;
	printk(KERN_DEBUG "reading fork counter\n");
	if(*ppos > 0 || count < BUFSIZE)
		return 0;
	len += sprintf(buf, "fork counter = %d\n", fork_counter);
	
	if(copy_to_user(ubuf, buf, len))
		return -EFAULT;
	*ppos = len;
	return len;
}

static struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.read = read_fork,
	.write = reset_fork,
};

static int simple_init(void)
{
	printk(KERN_ALERT "Khalifa in...\n");
	ent=proc_create("fork_count", 0660, NULL, &myops);
	return 0;
}

static void simple_cleanup(void)
{
	proc_remove(ent);
	printk(KERN_ALERT "Khalifa out...\n");
}

module_init(simple_init);
module_exit(simple_cleanup);

// Below is the code from exercise 7 for extracting fork count value
/*
void    **sys_call_table_ptr;
int counter = 0;
asmlinkage int (*old_fork)(void);

asmlinkage int new_fork(void)
{
    printk("Khalifa hooked\n");
    counter ++;
   // if(counter%10==0)
   // {
	printk("Fork counter = %d\n", counter);
   // }
    
    return old_fork();
}

static int __init myfork_init(void)
{
//	printk("Helloo...\n");/*
	//table address using kernel APIs
	unsigned long table_address = kallsyms_lookup_name("sys_call_table");
	printk("address --> %#lx \n", table_address);


	write_cr0(read_cr0() & (~0x10000)); //modifying cr0 register
    	sys_call_table_ptr = (void**)table_address; //setting pointer to table address
    	old_fork = sys_call_table_ptr[__NR_clone]; //saving original system call
    	sys_call_table_ptr[__NR_clone] = new_fork; //hooking my system call
    	write_cr0(read_cr0() | (0x10000)); //restoring cr0 register

	//  
	return 0;
}

static void __exit myfork_exit(void)
{
//	printk("Bye...\n");/*
        write_cr0(read_cr0() & (~0x10000)); //modifying cr0 register
    	sys_call_table_ptr[__NR_clone] = old_fork; //restoring original system call
    	write_cr0(read_cr0() | (0x10000)); //restoring cr0 register	
//
}

module_init(myfork_init);
module_exit(myfork_exit);
*/
