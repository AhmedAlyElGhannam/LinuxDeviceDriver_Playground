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
#include <linux/property.h>
#include <linux/of.h>
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

/* defining pointers to private device && private driver data */
static struct platform_driver_private_data* _prvtDrvData;
// static struct platform_device_private_data* _prvDevData;

/* defining an instance of file operations provided by this driver */
static const struct file_operations gpio_fops = 
{
    .owner = THIS_MODULE,
    .open = gpio_open,
    .write = gpio_write,
    // .read = gpio_read
};

/* each platform device is given a unique id to handle their *undiscoverable* nature */
const struct platform_device_id _id_table[NUM_OF_GPIOS] = 
{
    [0] = { .name = "LED_AQUA" }, /* .driver_data field is ignored */
    [1] = { .name = "LED_CRIMSON" } /* .driver_data field is ignored */
};

// will be used with device tree
// MODULE_DEVICE_TABLE(of, my_platform_dt_ids);

/* open callback function */
static int gpio_open(struct inode *inode, struct file *file) 
{
    /* extract device private data from passed pointer to inode */
    struct platform_device_private_data* _prvDevData = container_of(inode->i_cdev, struct platform_device_private_data, _cdev);
    
    /* set device private data into passed file private data*/
    file->private_data = _prvDevData;
    
    /* log before exiting */
    printk("Open CBF for LED GPIO device/file number %d\n", _prvDevData->pin_num);

    return 0;
}

/* write callback function */
static ssize_t gpio_write(struct file* file, const char __user* user_buff, size_t len, loff_t* off) 
{
    u32 not_copied;
    ssize_t res = 0;
    char buffer[GPIO_BUF_SIZE] = {0};

    /* extracting device private data from passed file pointer's private data field */
    struct platform_device_private_data* _prvDevData = (struct platform_device_private_data*) file->private_data;
    if (_prvDevData == NULL)
    {
        /* exit if there is no data */
        res = -ENODATA;
        goto out;
    }    

    /* NOTE: this field is still under construction */
    // unsigned char* buffer = (unsigned char*)_prvDevData->_gpio.allocatedMem;

    // printk("Write CBF for LED GPIO device/file number %d\n", priv->_gpio.pin_num);

    // /* Get amount of data to copy */
    // // to_copy = min(count, buffer_pointer);
    // printk("%s: the count to write %d \n", __func__, len); // 20
    // printk("%s: the offs %lld \n", __func__, *off);          // 0
    // if (len + *off > priv->_gpio.buffSize)   // count 10 off 3 size 9
    // {
    //     len = priv->_gpio.buffSize - *off; // 15
    // }
    // else {}

    // if (!len)
    // {
    //     printk("No space left!\n");
    //     res = -EFAULT;
    //     goto out;
    // }
    // else {}

    /* copy used input into local buffer */
    not_copied = copy_from_user(buffer, user_buff, len);
    if (not_copied)
    {
        printk("Buffer overflow!\n");
        res = -EFAULT;
        goto out;
    }
    else {}

    if (buffer[0] == '0')
    {
        /* for '0' -> log + clear pin */
        printk("Turning LED off\n");
        iowrite32(1 << _prvDevData->pin_num, _prvDevData->_gpio.GPCLR);
    }
    else if (buffer[0] == '1')
    {
        /* for '1' -> log + set pin */
        printk("Turning LED on\n");
        iowrite32(1 << _prvDevData->pin_num, _prvDevData->_gpio.GPSET);
    }
    else 
    {
        /* for unknown arg -> log and exit */
        printk("Invalid Argument! Enter 0 to turn LED off || 1 to turn LED on\n");
    }

    res = len;


    /* NOTE: this field is still under construction */
    // if ((priv->_gpio.permissions == PERM_WO) || (priv->_gpio.permissions == PERM_RW))
    // {
        // if (buffer[0] == '0')
        // {
        //     printk("Turning LED off\n");
        //     iowrite32(1 << _prvDevData->pin_num, _prvDevData->_gpio.GPCLR);
        // }
        // else if (buffer[0] == '1')
        // {
        //     printk("Turning LED on\n");
        //     iowrite32(1 << _prvDevData->pin_num, _prvDevData->_gpio.GPSET);
        // }
        // else 
        // {
        //     printk("Invalid Argument! Enter 0 to turn LED off || 1 to turn LED on\n");
        // }

        // res = len;
    // }
    // else /* read only */
    // {
    //     res = 0;
    //     printk("Device has Read-only permission!\n");
    // }

out:
    return res;
}

/* probing callback function */
int probeCBF (struct platform_device* platResource)
{
    printk("from probe function\n");
    return 0;
}

int removeCBF (struct platform_device* platResource)
{   
    printk("from remove function\n");
    return 0;
}

struct myprivateData {
    int a;
    int b;
};
struct myprivateData element;

const struct of_device_id mydeviceId = {
    .compatible = "LED_AQUA",
    .data = &element,
    .name = "my led device",
    .type = "character"
};

/* platform driver instance */
struct platform_driver _platDriver = 
{
    .probe = probeCBF, /*probe callback function*/
    .remove = removeCBF, /* remove callback function */
    .driver = {
        .name = DRIVER_NAME, /* name of driver */
        .owner = THIS_MODULE /* owner of this driver */,
        .of_match_table = &mydeviceId
    }
};

static int __init gpioDriv_init(void)
{
    int res = 0;

    
    /* allocate memory for private driver data and initialize it to 0 */
    _prvtDrvData = kzalloc(sizeof(struct platform_driver_private_data), GFP_KERNEL);
    if (!_prvtDrvData)
    {
        res = -ENOMEM;
        printk("_prvtDrvData err\n");
        goto out;
    }

    /* reserve a major number (+ mem) for character device and its entries */
    res = alloc_chrdev_region(&_prvtDrvData->devNum, 0, NUM_OF_ENTRIES, DRIVER_NAME);
    if (res < 0)
    {
        res = -ENOMEM;
        printk("err_chrdev\n");
        goto err_chrdev;
    }

    /* create class for platform driver */
    _prvtDrvData->_class = class_create(THIS_MODULE, DRIVER_NAME);
    if (!_prvtDrvData->_class) 
    {
        res = PTR_ERR(_prvtDrvData->_class);
        printk("err_class\n");
        goto err_class;
    }

    /* remap base gpio address in order to access it */
    _prvtDrvData->gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
    if (!_prvtDrvData->gpio_base) 
    {
        res = -ENOMEM;
        printk("err_ioremap\n");
        goto err_ioremap;
    }

    /* register platform driver */
    res = platform_driver_register(&_platDriver);
    if (res < 0)
    {
        printk("err_platform\n");
        goto err_platform;
    }

    /* all is well */
    printk("Driver Initialization Successful!\n");
    goto out;

    /* labels for different error actions */
err_platform:
    iounmap(_prvtDrvData->gpio_base);
err_ioremap:
    class_destroy(_prvtDrvData->_class);
err_class:
    unregister_chrdev_region(_prvtDrvData->devNum, NUM_OF_ENTRIES);
err_chrdev:
    kfree(_prvtDrvData);
out:
    return res;
}

static void __exit gpioDriv_exit(void)
{
    /* unregister platform driver */
    platform_driver_unregister(&_platDriver);

    /* unmap gpio base register address */
    iounmap(_prvtDrvData->gpio_base);

    /* destroy driver class */
    class_destroy(_prvtDrvData->_class);

    /* unregister char dev (unmark its reserved region + major number) */
    unregister_chrdev_region(_prvtDrvData->devNum, NUM_OF_ENTRIES);

    /* free allocated private driver data */
    kfree(_prvtDrvData);
}

module_init(gpioDriv_init); 
module_exit(gpioDriv_exit); 