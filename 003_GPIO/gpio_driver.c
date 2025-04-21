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
static int gpio_open(struct inode *inode, struct file *file);
static ssize_t gpio_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
// static ssize_t gpio_read(struct file *, char __user *, size_t, loff_t *);
int probeCBF(struct platform_device* platResource);
int removeCBF(struct platform_device* platResource);

static const struct file_operations gpio_fops = 
{
    .owner = THIS_MODULE,
    .open = gpio_open,
    .write = gpio_write,
    // .read = gpio_read
};

struct platform_driver_private_data
{
    dev_t devNum; /* major + base minor */
    struct class* _class; /* driver class in sys */
    int numOfEntries; /* device count */
};  

struct platform_device_private_data
{
    struct cdev _cdev;      /* logical representation of character devices */
    struct device* _device; /* physical representation of devices (in general) in Linux */
    struct GPIO _gpio;      /* hardware-level info about gpio (registers + pins) */
};

static struct platform_driver_private_data _prvtDrvData;
static struct platform_device_private_data _prvDevData;

/* each platform device is given a unique id to handle their *undiscoverable* nature */
const struct platform_device_id _id_table[NUM_OF_GPIOS] = 
{
    [0] = {
        .name = "LED_AQUA"
        /* .driver_data if ignored */
    },
    [1] = {
        .name = "LED_CRIMSON"
        /* .driver_data if ignored */
    }
};

// will be used with device tree
// MODULE_DEVICE_TABLE(of, my_platform_dt_ids);



static int gpio_open(struct inode *inode, struct file *file) 
{
    struct platform_device_private_data* priv = container_of(inode->i_cdev, struct platform_device_private_data, _cdev);
    file->private_data = priv;
    printk("Open CBF for LED GPIO device/file number %d\n", priv->_gpio.pin_num);
    return 0;
}

static ssize_t gpio_write(struct file* file, const char __user* user_buff, size_t len, loff_t* off) 
{
    u32 not_copied;
    ssize_t res = 0;

    struct platform_device_private_data* priv = file->private_data;
    if (!priv)
    {
        res = -ENODATA;
        goto out;
    }    
    unsigned char* buffer = (unsigned char)priv->_gpio.allocatedMem;

    printk("Write CBF for LED GPIO device/file number %d\n", priv->_gpio.pin_num);

    /* Get amount of data to copy */
    // to_copy = min(count, buffer_pointer);
    printk("%s: the count to write %d \n", __func__, len); // 20
    printk("%s: the offs %lld \n", __func__, *off);          // 0
    if (len + *off > priv->_gpio.buffSize)   // count 10 off 3 size 9
    {
        len = priv->_gpio.buffSize - *off; // 15
    }
    else {}

    if (!len)
    {
        printk("No space left!\n");
        res = -EFAULT;
        goto out;
    }
    else {}

    not_copied = copy_from_user(&buffer[*off], user_buff, len);
    if (not_copied)
    {
        printk("Buffer overflow!\n");
        res = -EFAULT;
        goto out;
    }
    else {}

    if ((priv->_gpio.permissions == PERM_WO) || (priv->_gpio.permissions == PERM_RW))
    {
        if (buffer[0] == '0')
        {
            printk("Turning LED off\n");
            iowrite32(1 << priv->_gpio.pin_num, priv->_gpio.GPCLR);
        }
        else if (buffer[0] == '1')
        {
            printk("Turning LED on\n");
            iowrite32(1 << priv->_gpio.pin_num, priv->_gpio.GPSET);
        }
        else 
        {
            printk("Invalid Argument! Enter 0 to turn LED off || 1 to turn LED on\n");
        }

        res = len;
    }
    else /* read only */
    {
        res = 0;
        printk("Device has Read-only permission!\n");
    }

out:
    return res;
}

int probeCBF (struct platform_device* platResource)
{
    int res = 0;
    uint32_t val;
    uint32_t mask_shift;
    printk("Device Detected! \n");
    
    struct GPIO* _lc_gpio = (struct GPIO*) dev_get_drvdata(&platResource->dev);
    struct platform_private_device* _lc_plat_dev = (struct platform_private_device*) dev_get_platdata(&platResource->dev);
    
    _lc_gpio->allocatedMem = kzalloc(_lc_gpio->buffSize, GFP_KERNEL);
    if (!(_lc_gpio->allocatedMem))
    {
        res = -ENOMEM;
        goto out;
    }
    else {}

    /* setting up gpio addresses */
    _lc_gpio->GPSEL = ioremap(_lc_gpio->GPSEL, GPIO_SIZE);
    _lc_gpio->GPSET = ioremap(_lc_gpio->GPSET, GPIO_SIZE);
    _lc_gpio->GPCLR = ioremap(_lc_gpio->GPCLR, GPIO_SIZE);

    /* set up gpio pin modes */
    val = ioread32(_lc_gpio->GPSEL);

    /* mask and operation changes based on pin */
    if (_lc_gpio->pin_num == 21)
    {
        mask_shift = 3;
    }
    else if (_lc_gpio->pin_num == 26)
    {
        mask_shift = 18;
    }
    else {}

    /* clear GPIO pin fsel bits then make it output */
    val &= ~(0b111 << mask_shift); 
    val |= (0b001 << mask_shift);    
    /* write modified value in register */
    iowrite32(val, _lc_gpio->GPSEL);

    /* char device fields getting filled (aka filling in cdev instance) */
    cdev_init(&_lc_plat_dev->_cdev, &gpio_fops); /* fops */

    res = cdev_add(&_lc_plat_dev->_cdev, _prvtDrvData.devNum + platResource->id, 1); /* major + base minor (id is added to shift minor num from base) && num of devices (one at a time) */
    if (res < 0)
    {
        goto out;
    }
    else {}

    _lc_plat_dev->_device = device_create(_prvtDrvData._class, NULL, _prvtDrvData.devNum + platResource->id, NULL, "gpio%d", platResource->id);
    if (&_lc_plat_dev->_device == NULL)
    {
        res = -ENOMEM;
        goto err;
    }
    else {}

err:
    cdev_del(&_lc_plat_dev->_cdev);
out:
    return res;
}

int removeCBF (struct platform_device* platResource)
{    
    // pretty much the same as platResource->dev.platform_data
    struct GPIO* _lc_gpio = (struct GPIO*) dev_get_platdata(&platResource->dev); 
    struct platform_private_device* _lc_plat_dev = (struct platform_private_device*) dev_get_platdata(&platResource->dev);

    device_destroy(_prvtDrvData._class, _prvtDrvData.devNum + platResource->id);

    cdev_del(&(_lc_plat_dev->_cdev));

    kfree(_lc_gpio->allocatedMem);

    printk("Device Removed! \n");

    return 0;
}

struct platform_driver _platDriver = 
{
    .probe = probeCBF,
    .remove = removeCBF,
    .id_table = _id_table,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE
    }
};

static int __init gpioDriv_init(void)
{
    int res = 0;

    res = alloc_chrdev_region(&_prvtDrvData.devNum, 0, NUM_OF_ENTRIES, DRIVER_NAME); 
    if (res < 0)
    {
        goto out;
    }
    else {}
    
    _prvtDrvData._class = class_create("device");
    if (!(_prvtDrvData._class))
    {
        res = -ENOMEM;
        goto err_class;
    }

    res = platform_driver_register(&_platDriver);
    if (res < 0)
    {
        goto err_plat;
    }
    else {}

err_plat:
    class_destroy(_prvtDrvData._class);

err_class:
    unregister_chrdev_region(_prvtDrvData.devNum, NUM_OF_ENTRIES);

out:
    return res;
}

static void __exit gpioDriv_exit(void)
{
    platform_driver_unregister(&_platDriver);
    class_destroy(_prvtDrvData._class);
    unregister_chrdev_region(_prvtDrvData.devNum, NUM_OF_ENTRIES);
}

module_init(gpioDriv_init); 
module_exit(gpioDriv_exit); 