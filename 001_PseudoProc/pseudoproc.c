#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>

MODULE_LICENSE("GPL"); // has to be specified to allow using GPL-licensed code in kernel
MODULE_AUTHOR("Nemesis"); // this is my gaming alias
MODULE_DESCRIPTION("A module that creates a pseudo proc file"); // module description has to be clear and meme-free
MODULE_VERSION("1.0.0"); // module version based on development


ssize_t	readCBF(struct file *, char __user *, size_t, loff_t *);
ssize_t	writeCBF(struct file *, const char __user *, size_t, loff_t *);

struct proc_dir_entry* dirEntry = NULL;

ssize_t	readCBF(struct file *, char __user *, size_t, loff_t *)
{
    printk(KERN_INFO "R E A D I N G\n");
    return 0;
}

ssize_t	writeCBF(struct file *, const char __user *, size_t, loff_t *)
{
    printk(KERN_INFO "W R I T I N G\n");
    return 1;
}

const struct proc_ops my_proc_ops = 
{
    .proc_read = readCBF,
    .proc_write = writeCBF
};

static int __init proc_init(void) 
{
    /* create proc file under proc dir */
    dirEntry = proc_create("hehe", 0666, NULL, &my_proc_ops);
    printk(KERN_INFO "Bonjour!\n");
    return 0;
}

static void __exit proc_exit(void)
{
    proc_remove(dirEntry);
    printk(KERN_INFO "A D I O S\n");
}

module_init(proc_init); 
module_exit(proc_exit); 