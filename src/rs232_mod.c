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
#include <linux/sched.h>

// FOPS functions
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static void create_device_node(void);
static void remap_io(void);
static void init_controller(void);

static irqreturn_t irqh(int, void*, struct pt_regs*);

MODULE_LICENSE("GPL");

#define DEVICE_NAME "rs232"
#define SUCCESS 0
#define BUF_LEN 80

static int major;
static int Device_Open  =   0;
int irq_resp;

int fctrl = 0;
int baudrate = BAUD_DIVISOR; // default baudrate

struct file_operations fops = {
read: device_read,
write: device_write,
open: device_open,
release: device_release
};

struct kfifo rx_fifo;
wait_queue_head_t inq;

// module_param(baudrate, int, 0644);
// MODULE_PARM_DESC(baudrate, "Device baudrate")

static struct class *rs232_device_class = 0;

int init_module(void) {
	major = register_chrdev(0, DEVICE_NAME, &fops);

	if (major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}

    create_device_node();
    remap_io();
    init_controller();

    if( kfifo_alloc(&rx_fifo, 256, GFP_KERNEL) )
        return -ENOMEM;

    init_waitqueue_head(&inq);

    irq_resp = request_irq(UART_IRQ,
                               (irq_handler_t)(irqh),
                               IRQF_SHARED | IRQF_TRIGGER_RISING,
                               DEVICE_NAME,
                               (void*)(irqh)
                           );

	return 0;
}

void cleanup_module(void) {
    dev_t devno;
    devno = MKDEV(major, 0);

    device_destroy(rs232_device_class, devno);
    class_unregister(rs232_device_class);

	unregister_chrdev(major, DEVICE_NAME);
    free_irq(UART_IRQ, (void*)(irqh));

    kfifo_free(&rx_fifo);
}

static int device_open(struct inode *inode, struct file *file) {
	if (Device_Open) {
		return -EBUSY;
	}

	Device_Open++;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
	Device_Open--;

	module_put(THIS_MODULE);

	return 0;
}

static ssize_t device_read(struct file *f, char *c, size_t s, loff_t *off){
    char ch;
    int count = 0;
    int resp;

    resp = wait_event_interruptible( inq, kfifo_len( &rx_fifo) != 0 );
    while( (count < s) && kfifo_out( &rx_fifo, &ch, sizeof(char)) )
    {
        resp = copy_to_user(c+count, &ch, 1);
        count++;
        printk(KERN_INFO "reading... %c\n", ch);
    }

    return count;
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
    unsigned char input_char;
    int res;
    input_char = ioread8(UART_RX_DATA);

    wake_up_interruptible( &inq );
    res = kfifo_in(&rx_fifo, &input_char, 1);

    return IRQ_HANDLED;
}

void create_device_node()
{
    dev_t devno;
    devno = MKDEV(major, 0);
    rs232_device_class = class_create( THIS_MODULE, DEVICE_NAME );
    device_create(rs232_device_class, NULL, devno, NULL, DEVICE_NAME);
}

void remap_io()
{
	request_mem_region(UART_BASE_ADDRRESS, 0x20, DEVICE_NAME);
	ioremap(UART_BASE_ADDRRESS, 0x20);
}

void init_controller()
{
    int baudval;

    iowrite8(fctrl & LOOP_BIT, UART_MODEM_CTRL);

    iowrite8(DIVISOR_ACCESS_BIT, UART_LINE_CTRL);

    //baudval = ((UART_CLOCK/baudrate+8)/16) & 0xFFFF;
    iowrite8(BAUD_DIVISOR, UART_DLATCH_LO);
    iowrite8(BAUD_DIVISOR>>8, UART_DLATCH_HI);

    iowrite8(WORD_LENGTH_8, UART_LINE_CTRL);

    iowrite8(0x01, UART_INTR_EN);

    fctrl = ioread8(UART_FIFO_CTRL);
    iowrite8(fctrl | FIFO_ENABLE_BIT, UART_FIFO_CTRL);
}
