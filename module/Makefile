obj-m += hide_module.o
hide_module-objs = hide.o ioctl.o

KDIR  := /lib/modules/$(shell uname -r)/build

all:
	        $(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	        $(MAKE) -C $(KDIR) M=$(PWD) clean
