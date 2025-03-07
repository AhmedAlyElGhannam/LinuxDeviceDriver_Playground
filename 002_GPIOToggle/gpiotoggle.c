#include "BCM2837.h"
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>

MODULE_LICENSE("GPL"); // has to be specified to allow using GPL-licensed code in kernel
MODULE_AUTHOR("Nemesis"); // this is my gaming alias
MODULE_DESCRIPTION("A module that toggles GPIO pin 26 on RPi3B+"); // module description has to be clear and meme-free
MODULE_VERSION("1.0.0"); // module version based on development

#define LED_PIN     26 /* GPIO_PIN_26 (pin number 37 in RPi3B+) */
#define FSEL_OUTPUT 0b001 /* each pin has 3 bits in FSEL register for mode selection */
#define PIN_HIGH    1U
#define PIN_LOW     0U

/* all function prototypes */
static int __init gpio_init(void);
static void __exit gpio_exit(void);
ssize_t	readCBF(struct file *, char __user *, size_t, loff_t *);
ssize_t	writeCBF(struct file *, const char __user *, size_t, loff_t *);
int gpio_setPinDir(int pinNum, int dir);
int gpio_setPinVal(int pinNum, int val);


struct proc_dir_entry* dirEntry = NULL;

ssize_t	readCBF(struct file *, char __user *, size_t, loff_t *)
{
    printk(KERN_INFO "R E A D I N G\n");
    return 0;
}

ssize_t	writeCBF(struct file *, const char __user * user, size_t, loff_t *)
{
    printk(KERN_INFO "W R I T I N G\n");
    char buf[10] = {0};

    if(copy_from_user(buf, user, 10) > 0) 
    {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }
    
    if (buf[0] == 0)
    {
        gpio_setPinVal(LED_PIN, PIN_LOW);
    }
    else if (buf[0] == 1)
    {
        gpio_setPinVal(LED_PIN, PIN_HIGH);
    }
    else 
    {
        return -1;
    }

    return 1;
}

const struct proc_ops my_proc_ops = 
{
    .proc_read = readCBF,
    .proc_write = writeCBF
};


/* set pin direction */
int gpio_setPinDir(int pinNum, int dir)
{
    switch (pinNum)
    {
        case LED_PIN:
            if (dir == FSEL_OUTPUT)
            {
                 /* clear GPIO pin 26 fsel bits then make it output */
                (*(volatile unsigned int*)BCM2837_GPFSEL2) =  ((*(volatile unsigned int*)BCM2837_GPFSEL2) & ~(0b111 << 18)) | (FSEL_OUTPUT << 18); /* write 001 in bit 18,19,20 */
            }
            else {}
        break;

        default:
            return -1;
        break;
    }

    printk(KERN_INFO "Hola From Set Pin Direction!\n");
    return 0;
}

/* set pin val */
int gpio_setPinVal(int pinNum, int val)
{
    switch (pinNum)
    {
        case LED_PIN:
            if (val == PIN_HIGH)
            {
                /* write 1 to bit 26 to set GPIO26 (HIGH) */
                (*(volatile unsigned int*)BCM2837_GPSET0) |= (1U << LED_PIN);
            }
            else if (val == PIN_LOW)
            {
                /* write 1 to bit 26 to clear GPIO26 (LOW) */
                (*(volatile unsigned int*)BCM2837_GPCLR0) |= (1U << LED_PIN);
            }
            else {}
        break;

        default:
            return -1;
        break;
    }

    printk(KERN_INFO "Privet From Set Pin Value!\n");
    return 0;
}

static int __init gpio_init(void) 
{
    /* create proc file under proc dir */
    dirEntry = proc_create("hehe", 0666, NULL, &my_proc_ops);

    /* set GPIO pin to output */
    gpio_setPinDir(LED_PIN, FSEL_OUTPUT);

    printk(KERN_INFO "Bonjour!\n");
    return 0;
}

static void __exit gpio_exit(void)
{
    proc_remove(dirEntry);
    printk(KERN_INFO "A D I O S\n");
}

module_init(gpio_init); 
module_exit(gpio_exit); 