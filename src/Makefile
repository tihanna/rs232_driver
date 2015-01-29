obj-m := rs232_mod.o

PWD := $(shell pwd)
LINUX := /tmp/linux-3.4.29
CROSS := /scratch/e2lp/tools/gcc-linaro-arm-linux-gnueabihf-4.7-2013.01-20130125_linux/bin/arm-linux-gnueabihf-gcc-4.7.3


default:
	$(MAKE) -I $(KERNEL_DIR)/arch/arm/include/asm/ -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
tests:
	$(CROSS) -static rx_test.c -o rx_test
	$(CROSS) -static tx_test.c -o tx_test

clean:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
		rm rx_test -v
	rm tx_test -v
