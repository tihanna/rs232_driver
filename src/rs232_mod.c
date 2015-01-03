#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include "rs232_memory_map.h"
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kfifo.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static irqreturn_t irqh(int, void*, struct pt_regs*);

MODULE_LICENSE("GPL");

#define DEVICE_NAME "rs232_mod"
#define SUCCESS 0
#define BUF_LEN 80

static int major;
static int Device_Open  =   0;
static char msg[BUF_LEN];
static char *msg_ptr;
int irq_resp;

int fctrl = 0;

struct file_operations fops = {
read: device_read,
write: device_write,
open: device_open,
release: device_release
};

static struct class *rs232_device_class = 0;

int init_module(void) {
    dev_t devno;
    printk(KERN_INFO "Hello world\n");
	major = register_chrdev(0, DEVICE_NAME, &fops);
    printk(KERN_INFO "Major is: %d\n", major);

	if (major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", major);

		return major;
	}

    devno = MKDEV(major, 0);
    rs232_device_class = class_create( THIS_MODULE, DEVICE_NAME );
    device_create(rs232_device_class, NULL, devno, NULL, DEVICE_NAME);

    // create device node

	request_mem_region(UART_BASE_ADDRRESS, 0x20, DEVICE_NAME);
	ioremap(UART_BASE_ADDRRESS, 0x20);

    iowrite8(fctrl & LOOP_BIT, UART_MODEM_CTRL);

    iowrite8(DIVISOR_ACCESS_BIT, UART_LINE_CTRL);

    iowrite8(BAUD_DIVISOR, UART_DLATCH_LO);
    iowrite8(BAUD_DIVISOR>>8, UART_DLATCH_HI);

    iowrite8(WORD_LENGTH_8, UART_LINE_CTRL);

    iowrite8(0x01, UART_INTR_EN);

    irq_resp = request_irq(UART_IRQ,
                               (irq_handler_t)(irqh),
                               IRQF_SHARED | IRQF_TRIGGER_RISING,
                               DEVICE_NAME,
                               (void*)(irqh)
                           );

    printk(KERN_INFO "request irq resp: %d\n", irq_resp);

    fctrl = ioread8(UART_FIFO_CTRL);
    iowrite8(fctrl | FIFO_ENABLE_BIT, UART_FIFO_CTRL);

    iowrite8(65, UART_TX_DATA);

	return 0;
}

void cleanup_module(void) {
    dev_t devno;
	printk(KERN_INFO "Goodbye world\n");
    devno = MKDEV(major, 0);

    device_destroy(rs232_device_class, devno);
    class_unregister(rs232_device_class);

	unregister_chrdev(major, DEVICE_NAME);
    free_irq(UART_IRQ, (void*)(irqh));
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

    fctrl = ioread8(UART_INTR_ID);
    printk(KERN_INFO "Interrupt ID: %x\n", fctrl);

	printk(KERN_INFO "Nesto...\n");
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
	Device_Open--;

	module_put(THIS_MODULE);

	return 0;
}

/*
void timer_handler()
{
    // check fifo and buffer state
    // read into the buffer
    // anything to send?
    // send it via uart
}
*/

static ssize_t device_read(struct file *f, char *c, size_t s, loff_t *off){
    // check your buffer
    char buff[10];
    int i;

    for(i = 0; i < 5; i++) {
        buff[i] = ioread8(UART_RX_DATA);
    }

    buff[6] = 0;


    printk(KERN_INFO "reading... %s\n", buff);

    return 0;
}

static ssize_t device_write(struct file *f, const char *c, size_t s, loff_t *off){
    int i;
    for(i = 0; i < s; i++) {
        printk(KERN_INFO "writing...\n");
        iowrite8(c[i], UART_TX_DATA);
    }

    return s;
}

irqreturn_t irqh(int irq, void *dev_id, struct pt_regs *regs)
{
    printk(KERN_INFO "Interrupt happened\n");
    ioread8(UART_RX_DATA);
    return IRQ_HANDLED;
}
