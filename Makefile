#
#
#
obj-m := cdata.o

KERNEL_DIR := /lib/modules/2.6.24-26-generic/build
PWD := $(shell pwd)


default:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko .*cmd modules.* Module.* .tmp_versions *.mod.c *.out
