#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/kallsyms.h>

#define MAX_DEV 2

static struct proc_dir_entry *ent1;
static struct proc_dir_entry *ent2;


void rc4(unsigned char * p, unsigned char * k, unsigned char * c, int l)
{
	unsigned char s [256];
	unsigned char t [256];
	unsigned char temp;
	unsigned char kk;
	int i, j, x;
	for(i=0; i<256; i++)
	{
		j=(j+s[i]+t[i])%256;
		temp=s[i];
		s[i]=s[j];
		s[j]=temp;
	}

	i=j=-1;
	for(x=0; x<l; x++)
	{
		i = (i+1)%256;
		j = (j+s[i])%256;
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
		kk = (s[i]+s[j])%256;
		c[x] = p[x]^s[kk];
	}

	//printk(KERN_ALERT "%s\n",c);
}
static int mychardev_open(struct inode *inode, struct file *file);
static int mychardev_release(struct inode *inode, struct file *file);
static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);
static ssize_t myproc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos);
static ssize_t myproc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos);

static const struct file_operations mychardev_fops = {
    .owner      = THIS_MODULE,
    .open       = mychardev_open,
    .release    = mychardev_release,
    .read       = mychardev_read,
    .write       = mychardev_write
};
static  struct file_operations cipher_proc_ops = 
{
	.owner = THIS_MODULE,
	.read = myproc_read
};
static struct file_operations key_proc_ops = 
{
	.owner = THIS_MODULE,
	.write = myproc_write
};


struct mychar_device_data {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *mychardev_class = NULL;
static struct mychar_device_data mychardev_data[MAX_DEV];

static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init mychardev_init(void)
{
    int err, i;
    dev_t dev;

    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "mychardev");

    dev_major = MAJOR(dev);

    mychardev_class = class_create(THIS_MODULE, "mychardev");
    mychardev_class->dev_uevent = mychardev_uevent;

    cdev_init(&mychardev_data[0].cdev, &mychardev_fops);
    mychardev_data[0].cdev.owner = THIS_MODULE;
    cdev_add(&mychardev_data[0].cdev, MKDEV(dev_major, 0), 1);
    device_create(mychardev_class, NULL, MKDEV(dev_major, 0), NULL, "cipher_key", 0);
    
    cdev_init(&mychardev_data[1].cdev, &mychardev_fops);
    mychardev_data[1].cdev.owner = THIS_MODULE;
    cdev_add(&mychardev_data[1].cdev, MKDEV(dev_major, 1), 1);
    device_create(mychardev_class, NULL, MKDEV(dev_major, 1), NULL, "cipher", 1);

    //creating our two procfs files
    ent1 = proc_create("cipher_key", 0660, NULL, &key_proc_ops);
    ent2 = proc_create("cipher", 0660, NULL, &cipher_proc_ops);

    return 0;
}

static void __exit mychardev_exit(void)
{
    int i;

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(mychardev_class, MKDEV(dev_major, i));
    }

    class_unregister(mychardev_class);
    class_destroy(mychardev_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

    proc_remove(ent1);//remove /proc/cipher_key
    proc_remove(ent2);//remove /proc/cipher
}

static int mychardev_open(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device open\n");
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device close\n");
    return 0;
}

char my_string[4096]="TEST HELLO";
char my_cipher[4096]="TESTING CIPHER";
char my_key[128]="MYKEY";
char proc_key[128]="MYKEY"; //to be compared with my_key
static ssize_t mychardev_read(struct file *file, char __user *ubuf, size_t count, loff_t *offset)
{
	char buf[4096];
	char kbuf[128];
	int len = 0;
//	printk("Reading device (%d)\n", MINOR(file->f_path.dentry->d_inode->i_rdev));
	
	if(MINOR(file->f_path.dentry->d_inode->i_rdev)==0) //--> /dev/cipher_key
	{
		printk(KERN_ALERT "You cannot read the key\n");
		return 0;		
	}

	//else minor = 1 --> /dev/cipher
	if(*offset>0||count<4096)
		return 0;
	len += sprintf(buf, "%s", my_cipher);
	if(copy_to_user(ubuf, buf, len))
		return -EFAULT;
	*offset = len;
	return len;
}

static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    size_t maxdatalen = 4096, maxkeylen = 128, ncopied;
   // printk("Writing device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));
	
    if(MINOR(file->f_path.dentry->d_inode->i_rdev)==1){ //user writing msg to /dev/cipher
    	if (count < maxdatalen)
        	maxdatalen = count;
    	ncopied = copy_from_user(my_string, buf, maxdatalen);
    	
    	my_string[maxdatalen] = 0;
   // 	printk("Data from the user: %s\n", my_string);
	rc4(my_string, my_key, my_cipher, 128);//encryting input data
	printk("Input message: %s\n", my_string);
	printk("Ciphered message:%s\n", my_cipher);
    }
    else{ //user writing key to /dev/cipher_key
	    if(count < maxkeylen)
		    maxkeylen = count;
	    ncopied = copy_from_user(my_key, buf, maxkeylen);
	
	    my_key[maxkeylen] = 0;
	    printk("MY KEY: %s\n", my_key);
    }
return count;
}

static ssize_t myproc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	size_t maxkeylen = 128, ncopied;
	if(count<maxkeylen)
		maxkeylen = count;
	ncopied = copy_from_user(proc_key, ubuf, maxkeylen);
	proc_key[maxkeylen] = 0;
	printk("PROC KEY: %s\n", proc_key);
	return count;
}
static ssize_t myproc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[4096];
	int len = 0;
//printk(KERN_ALERT "PROC KEY = %s\n", proc_key);
//printk(KERN_ALERT "MY KEY = %s\n", my_key);
	if(strcmp(proc_key, my_key)==0)
	{
		if(*ppos>0||count<4096)
			return 0;
		len += sprintf(buf, "%s", my_string);
		if(copy_to_user(ubuf, buf, len))
			return -EFAULT;
		*ppos = len;
		return len;
	}
	else{
		if(*ppos>0||count<4096)
			return 0;
		len += sprintf(buf, "%s", my_cipher);
		if(copy_to_user(ubuf, buf, len))
			return -EFAULT;
		*ppos = len;
		return len;
	}
}
	
MODULE_LICENSE("GPL");

module_init(mychardev_init);
module_exit(mychardev_exit);
