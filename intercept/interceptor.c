#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/errno.h> 
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>  
#include <asm/page.h>  
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
 
unsigned long *syscall_table = NULL; 
 
asmlinkage int (*original_write)(unsigned int, const char __user *, size_t);
 
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
 
asmlinkage int new_write(unsigned int fd, const char __user *buf, size_t count) {
 
    // hijacked write
 
    printk(KERN_ALERT "WRITE HIJACKED");
 
    return (*original_write)(fd, buf, count);
}
 
static int init(void) {
 
    printk(KERN_ALERT "\nHIJACK INIT\n");
   
    write_cr0 (read_cr0 () & (~ 0x10000));
 
syscall_table= aquire_sys_call_table();
 
    original_write = (void *)syscall_table[__NR_write];
    syscall_table[__NR_write] = new_write;  
 
    write_cr0 (read_cr0 () | 0x10000);
 
    return 0;
}
 
static void exit(void) {
 
    write_cr0 (read_cr0 () & (~ 0x10000));
 
    syscall_table[__NR_write] = original_write;  
 
    write_cr0 (read_cr0 () | 0x10000);
     
    printk(KERN_ALERT "MODULE EXIT\n");
 
    return;
}
 
module_init(init);
module_exit(exit);
