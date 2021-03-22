#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct myfile{
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;
};

struct myfile procfile;

static char buf[15] = "";

void open_file_for_read(char *filename){
	procfile.fp = filp_open(filename, O_RDONLY, 0644);
	procfile.fs = get_fs();
	set_fs(KERNEL_DS);
}
void read_from_file_until(struct myfile mf, char *buf, unsigned long vlen){
	mf.pos = vlen;
	vfs_read(mf.fp, buf, sizeof(buf)+7, &mf.pos);
}
void close_file(struct myfile mf){
	filp_close(mf.fp, NULL);
	set_fs(mf.fs);
}

int init_module1 (void){
	printk("KHALIFA READING PROC FILE...\n");
	
	open_file_for_read("/proc/version");
	
	read_from_file_until(procfile, buf, 14);
	
	printk("MY KERNEL VERSION --> %s\n", buf);
	
	close_file(procfile);
	
	return 0;
}

void exit_module1 (void){
	printk("...MODULE OUT\n");
}

module_init(init_module1);
module_exit(exit_module1);
MODULE_LICENSE("GPL");
