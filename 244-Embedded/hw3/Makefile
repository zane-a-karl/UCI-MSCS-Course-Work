obj-m:=nothing.o hello.o
all:
	make -C /lib/modules/5.15.74-v7+/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/5.15.74-v7+/build M=$(shell pwd) clean
