#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL"); // has to be specified to allow using GPL-licensed code in kernel
MODULE_AUTHOR("Nemesis"); // this is my gaming alias
MODULE_DESCRIPTION("A module that does X on RPi3B+"); // module description has to be clear and meme-free
MODULE_VERSION("1.0.0"); // module version based on development

/* all function prototypes */
static int __init gpioDev_init(void);
static void __exit gpioDev_exit(void);

#define GPIO_BASE
#define GPIO_GPFSEL
#define GPIO_GPSET
#define GPIO_GPCLR
#define NUM_OF_GPIOS

struct GPIO 
{
    u32 buffSize;
    void* __iomem GPSEL;
    void* __iomem GPSET;
    void* __iomem GPCLR;
    u32 permissions;
    u32 SEL_MASK;
    u32 SET_MASK;
    u32 CLR_MASK;
    u32 mode;
    void* allocatedMem;
};

struct GPIO _gpios[NUM_OF_GPIOS] = 
{
    [0] = {
        .buffSize = 50,
        .GPSEL = (void* __iomem)(GPIO_BASE + GPIO_GPFSEL),
        .GPSET = (void* __iomem)(GPIO_BASE + GPIO_GPFSEL),
        .GPCLR = (void* __iomem)(GPIO_BASE + GPIO_GPFSEL),
        .permissions = 1
    },
    [1] = {
        .buffSize = 50,
        .GPSEL = (void* __iomem)(GPIO_BASE + GPIO_GPFSEL),
        .GPSET = (void* __iomem)(GPIO_BASE + GPIO_GPFSEL),
        .GPCLR = (void* __iomem)(GPIO_BASE + GPIO_GPFSEL),
        .permissions = 2
    }
};

void releaseCBF(struct device* dev)
{

}

struct platform_device _platdevice[NUM_OF_GPIOS] = 
{
    [0] = {
        .name = "gpio",
        .id = 0,
        .dev = {
            .platform_data = (struct GPIO*)&_gpios[0],
            .release = releaseCBF
        }
    },
    [1] = {
        .name = "gpio",
        .id = 1,
        .dev = {
            .platform_data = (struct GPIO*)&_gpios[1],
            .release = releaseCBF
        }
    }
};

static int __init gpioDev_init(void)
{
    u32 iter;
    for (iter = 0; iter < NUM_OF_GPIOS; iter++)
    {
        platform_device_register(&_platdevice[iter]);
    }

    return 0;
}

static void __exit gpioDev_exit(void)
{
    u32 iter;
    for (iter = 0; iter < NUM_OF_GPIOS; iter++)
    {
        platform_device_unregister(&_platdevice[iter]);
    }
}

module_init(gpioDev_init); 
module_exit(gpioDev_exit); 