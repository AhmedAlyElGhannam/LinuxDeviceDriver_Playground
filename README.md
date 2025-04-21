# LinuxDeviceDriver_Playground

> Install latest kernel headers:
```bash
sudo apt install linux-headers-$(uname -r)
```

> add include paths to .vscode/c_cpp_properties.json


> module_init && module_exit &rarr; like constructor & destructor for module


> <linux/moduleparam> &rarr; header to give arguments to module while inserting it into kernel
```c
module_param(var_name, var_type, PERMISSION); // perm = 0666 (read/write for user/group/others) (give it 0 to make it invisible in your fs) (or use the permission macros in stat.h)
MODULE_PARAM_DESC(var_name, "description for var_name"); // gives description for the param (gets printed when using modinfo)
```

```bash
sudo insmod module.ko var_name=5
```

> <include/uapi/linux/stat.h> &rarr; this header contains the standard macros for permissions in linux
```c
// s => stat
// i =>inode
// r => read
// w => write
// usr => user
// oth => other
// grp => group
#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001
```

> command to know info about a module
```bash
modinfo module.ko
```

> Major number &rarr; number unique to a certain driver
> Minor number &rarr; number unique to each file communicating with said driver. (file represents device in this case) (all of these files are controlled with the same driver with its unique major number)

> <linux/fs.h> &rarr; contains a function that registers a major number to a driver MANUALLY (drivers are assigned a major number automatically via a function *see below*)
```c
struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .open = openCBF,
    .release = releaseCBF
    .read = readCBF,
    .write = writeCBF
};
register_chrdev(major_num, "driver_name_reserving_this_major_num", &fops); 
.
.
.
unregister_chrdev(major_num, "driver_name_reserving_this_major_num");
```

> `__FUNCTION__` &rarr; used with `printk` to print function name calling `printk`
```c
printk("%s\n", __FUNCTION__);
```

> `alloc_chrdev_region` &rarr; finds a vacant major number and reserves it for this driver
```c
// the function takes an empty `dev` and fills it by itself (kinda returns it)
alloc_chrdev_region(dev_t* dev, unsigned baseminor, unsigned count, const char* name);
// dev => u32 variable containing both major number for driver && FIRST minor number
// baseminor => starting minor num 
// count => number of devices/files representing devices
// name => name of driver
.
.
.
dev_t dev;
alloc_chrdev_region(&dev, 0, 4, "driver_name");
.
.
.
.
unregister_chrdev_region(dev, 4); // 4 here is the number of devices
```

### defining driver as char device driver
1. cdev_init()
1. cdev_add()

> <linux/cdev.h> &rarr; must be included in order to use the functions above

```c
struct cdev chrdev = 
{
    .dev = // dev_t aka major && first minor number
    .count = // number of devices/files controlled by this driver
    .ops = // ptr to file operations 
    .kobj = // tbc
    .owner = // tbc
};
// VIMP: cdev passed to this function is EMPTY aka no need to fill its fields since the function itself does this for you
cdev_init(struct cdev*, const struct file_operations*);
// AGAIN, this function fills in the dev && count fields in cdev
res = cdev_add(&chrdev, dev, count);
if (res != ALL_OK)
{
    // delete the allocated major num in case an error happened
    unregister_chrdev_region(dev, count);
    return -1
}
```

### generating char dev driver class
1. class_create()
1. device_create() // creates the actual file(s)

> <linux/device.h> &rarr; must be included in this part

```c
// macro that returns pointer to class (just hold that address in a ptr, no need to fill any of the fields yourself)
// takes `this` and class name
struct class* _class = class_create(THIS_MODULE, "class_name");
/*
    MUST CHECK the return
    if (!_class)
    {
        // print err
        cdev_del(&chrdev);
        unregister_chrdev_region(dev, count);
        return -1;
    }

*/

// ptr to class
// parent class (null in this case)
// dev_t aka major+minor num
// data this driver has (0 in this case)
// device name
struct device* this_device = device_create(_class, NULL, dev_t dev, void drvdata, const char* fmt, ...);
/*
    MUST CHECK the return
    if (!_class)
    {
        // print err
        class_destroy(_class);
        cdev_del(&chrdev);
        unregister_chrdev_region(dev, count);
        return -1;
    }
*/
```

> `goto` is often used in linux to decrease duplicated code

```c
res = 0;
register_chrdev(major_num, "driver_name_reserving_this_major_num", &fops); 
// you get the idea
alloc_chrdev_region(&dev, 0, 4, "driver_name");
// you get the idea
cdev_init(struct cdev*, const struct file_operations*);
// you get the idea
res = cdev_add(&chrdev, dev, count);
if (err)
{
    goto err_cdevAdd;
}
struct class* _class = class_create(THIS_MODULE, "class_name");
if (err)
{
    goto err_classCreate;
}
struct device* this_device = device_create(_class, NULL, dev_t dev, void drvdata, const char* fmt, ...);
if (err)
{
    goto err_devCreate;
}
goto out;

err_devCreate:
    class_destroy();
err_classCreate:
err_cdevAdd:
    cdev_del(&chrdev);
err_cdevInit:
    unregister_chrdev_region(dev, 4); 
err_allocChr:
    unregister_chrdev()
err_regChrDev:
    res = -1;

out:
return res;

```


### dealing with memory (from/to user to/from kernel)
> `copy_from_user` (write) && `copy_to_user` (read)

### how to use this driver in c/c++/python?
> `fopen("/dev/device")` and write/read what you want

### error status
> <include/uapi/asm-generic/errno-base>
```c
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */
```