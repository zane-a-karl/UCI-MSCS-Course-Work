obj-m:=nothing.o
all:
	make -C /lib/modules/5.15.74+/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/5.15.74+/build M=$(shell pwd) clean
