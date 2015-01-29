obj-m := rs232_mod.o

PWD := $(shell pwd)

default:
	$(MAKE) -I $(KERNEL_DIR)/arch/arm/include/asm/ -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
