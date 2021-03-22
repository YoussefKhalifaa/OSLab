#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

struct myfile{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;
};

int flag = 0;

struct myfile procfile;

static char buf[0] = "";

void open_file_for_read(char *filename){
	procfile.fp = filp_open(filename, O_RDONLY, 0644);
	procfile.fs = get_fs();
	set_fs(KERNEL_DS);
}
void read_from_file_until(struct myfile mf, char *buf, unsigned long vlen){
	mf.pos = vlen;
	vfs_read(mf.fp, buf, sizeof(buf)+8, &mf.pos);
}
void close_file(struct myfile mf){
	filp_close(mf.fp, NULL);
	set_fs(mf.fs);
}

void script(void)
{
char *argv[] = 
{"/bin/bash","-c","/bin/cat /boot/System.map-4.19.0-13-amd64 | grep sys_call_table > /root/address.txt", NULL};
	char *envp[] = {"/", NULL};
	call_usermodehelper(argv[0], argv, envp, 2);
}

int init_module1 (void){
	unsigned long long base_address; //__NR_SYSCALL_BASE address

	printk("KHALIFA IN...\n");

	script(); //traversing System.map to find sys_call_table and storing it in address.txt

	open_file_for_read("address.txt"); //opening address.txt

	read_from_file_until(procfile, buf, 0); //reading address from address.txt

	close_file(procfile); //close address.txt

	base_address = simple_strtoull(buf, NULL, 16); //string address to unsigned LL

	printk("sys_call_table address --> 0x%s\n", buf); //sys_call_table address from System.map

//	printk("fork address --> %#llx", *((unsigned long long *)base_address+2));//__NR_fork address
	printk("fork address --> %#llx", base_address+2);

	return 0;
}

void exit_module1 (void){
	printk("...KHALIFA OUT\n");
}

module_init(init_module1);
module_exit(exit_module1);
MODULE_LICENSE("GPL");
