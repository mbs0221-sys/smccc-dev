# Makefile for smccc-module kernel module

obj-m += smccc-module.o

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
INSTALL_DIR ?= /lib/modules/$(shell uname -r)/extra
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
