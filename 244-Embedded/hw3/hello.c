#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("Dual BSD/GPL");
static int hello_init(void) {
	printk(" Pork chop sandwiches!\n");
	return 0;
}
static void hello_exit(void) {
	printk(" What are you doing? Get out of here!\n");
}
module_init(hello_init);
module_exit(hello_exit);
