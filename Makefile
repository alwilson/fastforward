obj-m := fastforward.o
KDIR := /lib/modules/$(shell uname -r)/build

CFLAGS_fastforward.o := -DDEBUG

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
