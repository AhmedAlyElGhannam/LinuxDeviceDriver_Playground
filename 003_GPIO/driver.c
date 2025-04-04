#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL"); // has to be specified to allow using GPL-licensed code in kernel
MODULE_AUTHOR("Nemesis"); // this is my gaming alias
MODULE_DESCRIPTION("A module that does X on RPi3B+"); // module description has to be clear and meme-free
MODULE_VERSION("1.0.0"); // module version based on development

/* all function prototypes */
static int __init gpioDriv_init(void);
static void __exit gpioDriv_exit(void);

#define NUM_OF_ENTRIES  5 // max num of devices this driver is able to operate

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
};

struct prv_data_device
{
    struct cdev _cdev;
    struct device* _device;
};

struct prv_data_driver 
{
    dev_t devNum;
    struct class* _class;
    int numOfEntries;
};  

int probeCBF (struct platform_device* platResource)
{
    printk("Device Detected! \n");
    struct GPIO* _lc_gpio = (struct GPIO*) platResource->dev.platform_data;
    _lc_gpio->allocatedMem = kzalloc(_lc_gpio->buffSize, GFP_KERNEL);

    /**
     * @brief 
     * set GPIOAFSEL mode
     * cdev_init
     * cdev_add
     * device_create
     * 
     */
    return 0
}

int removeCBF (struct platform_device* platResource)
{
    printk("Device Removed! \n");
    struct GPIO* _lc_gpio = (struct GPIO*) platResource->dev.platform_data;
    kfree(_lc_gpio->allocatedMem);
    return 0;
}

struct platform_driver _platDriver = 
{
    .probe = probeCBF,
    .remove = removeCBF,
    .driver = {
        .name = "_gpio"
    }
};

struct prv_data_driver _prvtDrvData;

static int __init gpioDriv_init(void)
{
    alloc_chrdev_region(&_prvtDrvData.devNum, 0, NUM_OF_ENTRIES, "device");
    _prvtDrvData._class = class_create("device");
    platform_driver_register(&_platDriver);
    return 0;
}

static void __exit gpioDriv_exit(void)
{
    unregister_chrdev_region(_prvtDrvData.devNum, NUM_OF_ENTRIES);
    class_destroy(_prvtDrvData._class);
    platform_driver_unregister(&_platDriver);
}

module_init(gpioDriv_init); 
module_exit(gpioDriv_exit); 