obj-m := rs232_mod.o

PWD := $(shell pwd)

default:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	cp rs232_mod.ko ../obj
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
