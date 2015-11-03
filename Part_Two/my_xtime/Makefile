TARGET := my_xtime

ifneq ($(KERNELRELEASE),)
	obj-m := my_xtime.o
else
	KERNELDIR ?= \
		/lib/modules/`uname -r`/build/
	PWD := `pwd`
default:
	$(MAKE) -C $(KERNELDIR) \
		M=$(PWD) modules
endif

clean:
	rm -f *.ko *.o Module* *mod*
