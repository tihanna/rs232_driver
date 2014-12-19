obj-m := rs232_mod.o

PWD := $(shell pwd)

default:
	cd src && make
clean:
	cd src && make clean
	rm obj/*

install:
	mkdir /opt/rs232_driver -vp
	cp scripts/*.sh /opt/rs232_driver/ -vf
	cp scripts/*.rules /etc/udev/rules.d/ -vf
	service udev restart

uninstall:
	rm -rv /opt/rs232_driver
	rm /etc/udev/rules.d/*rs232* -v

ld:
	insmod  obj/rs232_mod.ko
ud:
	rmmod rs232_mod -v
rld: ud ld
