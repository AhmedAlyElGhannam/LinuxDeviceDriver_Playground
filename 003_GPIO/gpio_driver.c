#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/mod_devicetable.h>
#include "gpio_driver.h"

MODULE_LICENSE("GPL"); // has to be specified to allow using GPL-licensed code in kernel
MODULE_AUTHOR("Nemesis"); // this is my gaming alias
MODULE_DESCRIPTION("A module that does X on RPi3B+"); // module description has to be clear and meme-free
MODULE_VERSION("1.0.0"); // module version based on development

/* all function prototypes */
static int __init gpioDriv_init(void);
static void __exit gpioDriv_exit(void);

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

const struct platform_device_id _id_table[NUM_OF_GPIOS] = 
{
    [0] = {
        .name = "Fady"
    },
    [1] = {
        .name = "Ahmed"
    }
};

struct prv_data_driver _prvtDrvData;
struct prv_data_device _prvDevData;

static int gpio_open(struct inode *inode, struct file *file) {
    struct prv_data_device *priv = container_of(inode->i_cdev, struct prv_data_device, cdev);
    file->private_data = priv;
    return 0;
}

static ssize_t gpio_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    struct prv_data_device *priv = file->private_data;
    u32 val;
    ssize_t res;

    if ((priv->gpio->permissions == PERM_WO) || (priv->gpio->permissions == PERM_RW))
    {
        if (copy_from_user(&val, buf, sizeof(val)))
            return -EFAULT;

        if (val)
            iowrite32(1 << priv->gpio->pin_number, priv->gpio->GPSET);
        else
            iowrite32(1 << priv->gpio->pin_number, priv->gpio->GPCLR);

        res = sizeof(val);
    }
    else /* read only */
    {
        res = 0;
        printk("Device has Read-only permission!\n");
    }

    return res;
}

static const struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .open = gpio_open,
    .write = gpio_write,
};

int probeCBF (struct platform_device* platResource)
{
    printk("Device Detected! \n");
    
    struct GPIO* _lc_gpio = (struct GPIO*) dev_get_drvdata(&platResource->dev);
    struct platform_private_device* _lc_plat_dev = (struct platform_private_device*) dev_get_platdata(&platResource->dev);
    
    _lc_gpio->allocatedMem = kzalloc(_lc_gpio->buffSize, GFP_KERNEL);

    // fills in cdev
    cdev_init(&_lc_plat_dev->cdev, &gpio_fops);
    cdev_add(&_lc_plat_dev->cdev, _prvtDrvData.devNum + platResource->id, 1); /* 1 is the version */
    
    device_create(_prvtDrvData._class, NULL, _prvtDrvData.devNum + platResource->id, NULL, "gpio%d", pdev->id);

    // _lc_gpio->GPSEL = ioremap(_lc_gpio->GPSEL, GPIO_SIZE);
    // _lc_gpio->GPSET = ioremap(_lc_gpio->GPSET, GPIO_SIZE);
    // _lc_gpio->GPCLR = ioremap(_lc_gpio->GPCLR, GPIO_SIZE);

    /**
     * @brief 
     * set GPIOAFSEL mode
     * cdev_init
     * cdev_add
     * device_create
     * 
     */
    return 0;
}

int removeCBF (struct platform_device* platResource)
{
    printk("Device Removed! \n");
    
    struct GPIO* _lc_gpio = (struct GPIO*) dev_get_platdata(&platResource->dev); // pretty much the same as platResource->dev.platform_data
    struct platform_private_device* _lc_plat_dev = (struct platform_private_device*) dev_get_platdata(&platResource->dev);

    kfree(_lc_gpio->allocatedMem);

    cdev_del(&(_lc_plat_dev->_cdev));
    device_destroy(_prvtDrvData._class, _prvtDrvData.devNum + platResource->id);

    return 0;
}

struct platform_driver _platDriver = 
{
    .probe = probeCBF,
    .remove = removeCBF,
    .id_table = _id_table,
    .driver = {
        .name = DRIVER_NAME
    }
};

static int __init gpioDriv_init(void)
{
    alloc_chrdev_region(&_prvtDrvData.devNum, 0, NUM_OF_ENTRIES, DRIVER_NAME); // fills 
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