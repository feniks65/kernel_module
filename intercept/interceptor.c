#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/errno.h> 
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>  
#include <asm/page.h>  
#include <linux/sched.h>
#include <linux/syscalls.h>

MODULE_LICENSE("GPL");

#define START_MEM 0xffffffff81a00000
#define END_MEM 0xffffffff81b00000

unsigned long long *syscall_table;


asmlinkage int (*org_mkdir)(const char *pathname, mode_t mode);

asmlinkage int hacked_mkdir(const char *pathname, mode_t mode)
{
	printk("MOJ MKDIR ZOSTAL URUCHIOMIONY!!!!!!!!!");
	
	return 0;
}

unsigned long long **find(void)
{
	unsigned long long **sctable;
	unsigned long long int i = START_MEM;

	while( i < END_MEM)
	{
		sctable = (unsigned long long **)i;

		if(sctable[__NR_close] == (unsigned long long *) sys_close)
		{
			return &sctable[0];
		}
		i += sizeof(void*);
	
	}
	return NULL;
} 
 
static int init(void) {
 
    printk(KERN_ALERT "\nHIJACK INIT\n");

    syscall_table = (unsigned long long *) find();

    if( syscall_table != NULL)
	printk("SysCall table found at %llx\n", (unsigned)syscall_table);
    else
	printk("Syscall table not found!\n");

    write_cr0 (read_cr0 () & (~ 0x10000));
 
    org_mkdir = syscall_table[__NR_mkdir];

    syscall_table[__NR_mkdir] = hacked_mkdir;  
 
    write_cr0 (read_cr0 () | 0x10000);
 
    return 0;
}
 
static void exit(void) {
 
    write_cr0 (read_cr0 () & (~ 0x10000));
 
    syscall_table[__NR_mkdir] = org_mkdir;  
 
    write_cr0 (read_cr0 () | 0x10000);
     
    printk(KERN_ALERT "MODULE EXIT\n");
 
    return;
}
 
module_init(init);
module_exit(exit);
