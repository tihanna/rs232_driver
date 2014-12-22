#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include "rs232_memory_map.h"

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

struct resource *request_mem_region(unsigned long start, unsigned long len,
                                    char *name);

#define DEVICE_NAME "rs232_mod"
#define SUCCESS 0
#define BUF_LEN 80

static int major;
static int Device_Open  =   0;
static char msg[BUF_LEN];
static char *msg_ptr;

struct file_operations fops = {
//read: device_read,
//write: device_write,
open: device_open,
release: device_release
};


int init_module(void) {
    printk(KERN_INFO "Hello world\n");
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", major);

		return major;
	}
	//alokacija i mapiranje
	request_mem_region(UART_BASE_ADDRRESS, 0x20, DEVICE_NAME);	
	ioremap(UART_BASE_ADDRRESS, 0x20);
	//podesavanje brzine	
	iowrite8(BAUD_DIVISOR & 0xFF, UART_DLATCH_LO);
	iowrite8(BAUD_DIVISOR << 8, UART_DLATCH_HI);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "Goodbye world\n");
	unregister_chrdev(major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file) {
	static int counter = 0;

	if (Device_Open) {
		return -EBUSY;
	}

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_ptr = msg;
	printk(KERN_INFO ":)))\n");
	try_module_get(THIS_MODULE);

	printk(KERN_INFO "Nesto...\n");
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
	Device_Open--;

	module_put(THIS_MODULE);

	return 0;
}

