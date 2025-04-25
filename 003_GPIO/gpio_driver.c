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
    int res = 0;
    uint32_t val;
    uint32_t mask_shift;
    struct GPIO* _lc_gpio;

    /* probe logging */
    printk("Device Detected! \n");

    /* allocating memory for platform device private data */
    struct platform_device_private_data* _prvDevData = (struct platform_device_private_data*) devm_kzalloc(&_prvDevData, sizeof(struct platform_device_private_data), GFP_KERNEL);
    if (_prvDevData == NULL)
    {
        /* if no memory -> return */
        res = -ENOMEM;
        goto out;
    }

    /* set private device data into platform device's private data field */
    platform_set_drvdata(platResource, _prvDevData);

    /* get a pointer to device's gpio field */
    _lc_gpio = (struct GPIO*) (&_prvDevData->_gpio);
    
    /* NOTE: this part is ALSO under construction */
    /* move to open() + use it in write + free it in close() */
    // /* setting buffer size */
    // _lc_gpio->buffSize = GPIO_BUF_SIZE;

    // /* allocating memory for GPIO buffer */
    // _lc_gpio->allocatedMem = kzalloc(_lc_gpio->buffSize, GFP_KERNEL);
    // if (!(_lc_gpio->allocatedMem))
    // {
    //     res = -ENOMEM;
    //     goto out;
    // }
    // else {}

    /* setting up gpio addresses */
    _lc_gpio->GPSEL = _prvtDrvData->gpio_base + GPIO_GPFSEL2;
    _lc_gpio->GPSET = _prvtDrvData->gpio_base + GPIO_GPSET0;
    _lc_gpio->GPCLR = _prvtDrvData->gpio_base + GPIO_GPCLR0;

    /* set pin number based on device id */
    switch (platResource->id)
    {
        case 0:
            /* set pin num in private data */
            _prvDevData->pin_num = LED_AQUA_PIN;
            /* mask and operation changes based on pin */
            mask_shift = 3;

        break;

        case 1:
            /* set pin num in private data */
            _prvDevData->pin_num = LED_CRIMSON_PIN;
            /* mask and operation changes based on pin */
            mask_shift = 18;
        break;

        default:
            res = -EINVAL;
        break;
    }

    /* set up gpio pin modes */
    val = ioread32(_lc_gpio->GPSEL);

    /* clear GPIO pin fsel bits then make it output */
    val &= ~(0b111 << mask_shift); 
    val |= (0b001 << mask_shift);
        
    /* write modified value in register */
    iowrite32(val, _lc_gpio->GPSEL);

    /* char device fields getting filled (aka filling in cdev instance) */
    cdev_init(&_prvDevData->_cdev, &gpio_fops); /* fops */

    res = cdev_add(&_prvDevData->_cdev, _prvtDrvData->devNum + platResource->id, 1); /* major + base minor (id is added to shift minor num from base) && num of devices (one at a time) */
    if (res < 0)
    {
        goto out;
    }
    else {}

    _prvDevData->_device = device_create(_prvtDrvData->_class, NULL, _prvtDrvData->devNum + platResource->id, NULL, "gpio-led%d", platResource->id);
    if (&_prvDevData->_device == NULL)
    {
        res = -ENOMEM;
        goto err;
    }
    else {}


    /* log successful exit of probe */
    printk("Device Probing Successful!\n");
    goto out;

err:
    cdev_del(&_prvDevData->_cdev);
out:
    return res;
}

int removeCBF (struct platform_device* platResource)
{   
    /* extracting device private data from passed device pointer */
    struct platform_device_private_data* _prvDevData = platform_get_drvdata(platResource);

    /* destroy the created platform device */
    device_destroy(_prvtDrvData->_class, _prvtDrvData->devNum + platResource->id);

    /* delete the created character device */
    cdev_del(&(_prvDevData->_cdev));

    /* NOTE: move it to close */
    // kfree(_lc_gpio->allocatedMem);

    /* logging */
    printk("Device Removed! \n");

    return 0;
}

/* platform driver instance */
struct platform_driver _platDriver = 
{
    .probe = probeCBF, /*probe callback function*/
    .remove = removeCBF, /* remove callback function */
    .id_table = _id_table, /* pointer to id table */
    .driver = {
        .name = DRIVER_NAME, /* name of driver */
        .owner = THIS_MODULE /* owner of this driver */
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