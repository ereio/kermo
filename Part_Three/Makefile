MODULE_NAME = elevator
PWD := $(shell pwd)

#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /lib/modules/4.2.0/build
ccflags-y += -I$(src)/include

obj-y := src/sys_start_elevator.o
obj-y += src/sys_issue_request.o
obj-y += src/sys_stop_elevator.o

$(MODULE_NAME)-objs += src/syscalls.o
$(MODULE_NAME)-objs += src/module.o
obj-m := $(MODULE_NAME).o

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
