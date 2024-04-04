obj-m += revshell.o
all:
	make -C /lib/modules/5.9.0-050900-generic/build M=$(PWD) modules
clean:
	make -C /lib/modules/5.9.0-050900-generic/build M=$(PWD) clean