#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

static int iter = 3;
module_param(iter,int,0660);
static char *msg = "This is CSCE-3402 kernel module";
module_param(msg,charp,0660);


static int hello_world_init_function (void) {
	//printk(KERN_INFO "HELLO WORLD CSCE-3402 :]\n");
	int i = 0;
	printk(KERN_INFO "Number of iterations --> %d\n", iter);
	for(i=0;i<iter;i++){
	printk(KERN_INFO "%s\n", msg);
	}
	return 0;
}

module_init(hello_world_init_function);

static void hello_world_cleanup_function (void) {
	printk(KERN_INFO "BYE BYE WORLD CSCE-3402 :]\n");
}

module_exit(hello_world_cleanup_function);
