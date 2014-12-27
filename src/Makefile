obj-m := rs232_mod.o

PWD := $(shell pwd)
LINUX := /tmp/e2lp_prerequisites/kernel/linux-3.4.29

default:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -I $(LINUX)/arch/arm/include/asm/ -C $(LINUX) M=$(PWD) modules
	cp rs232_mod.ko ../obj
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
