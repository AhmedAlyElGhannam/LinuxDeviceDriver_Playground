#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include "gpio_driver.h"

MODULE_LICENSE("GPL"); // has to be specified to allow using GPL-licensed code in kernel
MODULE_AUTHOR("Nemesis"); // this is my gaming alias
MODULE_DESCRIPTION("A module that does X on RPi3B+"); // module description has to be clear and meme-free
MODULE_VERSION("1.0.0"); // module version based on development

/* all function prototypes */
static int __init gpioDev_init(void);
static void __exit gpioDev_exit(void);

struct GPIO _gpios[NUM_OF_GPIOS] = 
{
    [0] = {
        .buffSize = 50,
        .GPSEL = (void* __iomem)(GPIO_GPFSEL0),
        .GPSET = (void* __iomem)(GPIO_GPSET0),
        .GPCLR = (void* __iomem)(GPIO_GPCLR0),
        .permissions = PERM_RW,
        .pin_num = 21
    },
    [1] = {
        .buffSize = 50,
        .GPSEL = (void* __iomem)(GPIO_GPFSEL0),
        .GPSET = (void* __iomem)(GPIO_GPSET0),
        .GPCLR = (void* __iomem)(GPIO_GPCLR0),
        .permissions = PERM_RW,
        .pin_num = 26
    }
};

void releaseCBF(struct device* dev)
{

}

struct platform_private_device _platform_private_device[NUM_OF_GPIOS];


struct platform_device _platdevice[NUM_OF_GPIOS] = 
{
    [0] = {
        .name = "fady",
        .id = 0,
        .dev = {
            .driver_data = (struct GPIO*)&_gpios[0],
            .platform_data = (struct platform_private_device*)&_platform_private_device[0],
            .release = releaseCBF
        }
    },
    [1] = {
        .name = "ahmed",
        .id = 1,
        .dev = {
            .driver_data = (struct GPIO*)&_gpios[1],
            .platform_data = (struct platform_private_device*)&_platform_private_device[1],
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