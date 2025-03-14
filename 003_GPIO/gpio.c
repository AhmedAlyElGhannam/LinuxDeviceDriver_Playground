#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL"); // has to be specified to allow using GPL-licensed code in kernel
MODULE_AUTHOR("Nemesis"); // this is my gaming alias
MODULE_DESCRIPTION("A module that does X on RPi3B+"); // module description has to be clear and meme-free
MODULE_VERSION("1.0.0"); // module version based on development

/* all function prototypes */
static int __init gpio_init(void);
static void __exit gpio_exit(void);

dev_t gpioDev;
#define BASENUM 0
#define DEV_FILE_COUNT  4
#define DEV_REGION_NAME "gpioDevRegion"
#define ALL_OK  0
uint32_t majorNum = 0; // marks which driver is used (assigned by the kernel)
uint32_t minorNum = 0; // marks which device is used

ssize_t writeCBF (struct file * file, const char __user * buf, size_t size, loff_t * loff)
{
    printk(KERN_INFO "W R I T I N G\n");
    return size;
}

static const struct file_operations fOps = 
{
    .write = writeCBF
    // .read = readCBF,
    // .open = openCBF,
    // .release = releaseCBF // to close a file
};

/* character device object */
static struct cdev GPIO = 
{
    .owner = THIS_MODULE
};

static int __init gpio_init(void) 
{
    int error_status = ALL_OK;

    /* allocate mem region for devices' files*/
    alloc_chrdev_region(&gpioDev, BASENUM, DEV_FILE_COUNT, DEV_REGION_NAME);
    
    /* extracting major and minor numbers from gpioDev */
    majorNum = MAJOR(gpioDev);
    minorNum = MINOR(gpioDev);
    printk("Major = %d, Minor = %d\n", majorNum, minorNum);

    /* initialize the char device object with its file operations */
    cdev_init(&GPIO, &fOps);

    /*int cdev_add(struct cdev *, dev_t, unsigned)*/
    error_status = cdev_add(&GPIO, gpioDev, DEV_FILE_COUNT); 
    if (error_status != 0)
    {
        error_status = -EAGAIN;
        goto out;
    }

    // class_create()
    // device_create()

    printk(KERN_INFO "Bonjour!\n");

out:
    return error_status;
}

static void __exit gpio_exit(void)
{
    /* delete char device object */
    cdev_del(&GPIO);

    /* deallocate char device region */
    unregister_chrdev_region(gpioDev, DEV_FILE_COUNT);
    printk(KERN_INFO "A D I O S\n");
}

module_init(gpio_init); 
module_exit(gpio_exit); 