#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#define GPIO_BASE   (0x3F200000UL)
#define GPIO_GPFSEL0 (GPIO_BASE)
#define GPIO_GPFSEL1 (GPIO_BASE + 0x04)
#define GPIO_GPFSEL2 (GPIO_BASE + 0x08)
#define GPIO_GPSET0 (GPIO_BASE + 0x1C)
#define GPIO_GPCLR0 (GPIO_BASE + 0x28)
#define GPIO_SIZE	    (0xA0)
#define NUM_OF_GPIOS 2

#define PERM_RO 1U
#define PERM_WO 2U
#define PERM_RW 3U

#define NUM_OF_ENTRIES  5 // max num of devices this driver is able to operate


struct GPIO 
{
    u32 buffSize;
    void* __iomem GPSEL;
    void* __iomem GPSET;
    void* __iomem GPCLR;
    u32 permissions;
    // u32 SEL_MASK;
    // u32 SET_MASK;
    // u32 CLR_MASK;
    // u32 mode;
    u32 pin_num;
    void* allocatedMem;
};

#endif