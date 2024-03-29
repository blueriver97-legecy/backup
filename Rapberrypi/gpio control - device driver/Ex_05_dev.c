#include <linux/module.h>    
#include <linux/kernel.h>    
#include <linux/cdev.h>    
#include <linux/device.h>    
#include <linux/fs.h>              
#include <linux/uaccess.h>    
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/io.h>
#include "Ex_05.h"

dev_t id;    
struct cdev cdev;    
struct class *class;    
struct device *dev;    
LedCtl ledCtl;

// GPIO 
typedef struct {
        unsigned long   GPFSEL[6];      ///< Function selection registers.
        unsigned long   Reserved_1;
        unsigned long   GPSET[2];
        unsigned long   Reserved_2;
        unsigned long   GPCLR[2];
        unsigned long   Reserved_3;
        unsigned long   GPLEV[2];
        unsigned long   Reserved_4;
        unsigned long   GPEDS[2];
        unsigned long   Reserved_5;
        unsigned long   GPREN[2];
        unsigned long   Reserved_6;
        unsigned long   GPFEN[2];
        unsigned long   Reserved_7;
        unsigned long   GPHEN[2];
        unsigned long   Reserved_8;
        unsigned long   GPLEN[2];
        unsigned long   Reserved_9;
        unsigned long   GPAREN[2];
        unsigned long   Reserved_A;
        unsigned long   GPAFEN[2];
        unsigned long   Reserved_B;
        unsigned long   GPPUD[1];
        unsigned long   GPPUDCLK[2];
        //Ignoring the reserved and test bytes
} BCM2837_GPIO_REGS;

#define BCM2837_GPIO_BASE 0x3F200000
#define BCM2837_GPIO_SIZE sizeof(BCM2837_GPIO_REGS)
void __iomem *mem = NULL;
volatile BCM2837_GPIO_REGS * pRegs = NULL;

#define DEVICE_NAME "Ex_05_dev"    
    
int simple_open (struct inode *inode, struct file *filp)    
{    
    printk( "open\n" );    
    return 0;    
}    
    
int simple_close (struct inode *inode, struct file *filp)    
{    
    printk( "close\n" );    
    return 0;    
}    
        
ssize_t simple_read(struct file *filp, char *buf, size_t size, loff_t *offset)    
{    
    printk( "read\n" );
    return 0;    
}    

ssize_t simple_write (struct file *filp, const char *buf, size_t size, loff_t *offset)    
{    
    printk( "write\n" );    
    return 0;    
}    
        
long simple_ioctl ( struct file *filp, unsigned int cmd, unsigned long arg)    
{
    unsigned long offset = 0;
    unsigned long mask = 0;
    unsigned long val = 0;
    int item = 0;

    if(_IOC_TYPE(cmd) != MY_IOC_MAGIC)
        return -EINVAL;
    if(_IOC_NR(cmd) >= MY_IOC_MAXNR)
        return -EINVAL;

    switch(cmd){
        case MY_IOC_SET:
        {   
            if(copy_from_user((void *)&(ledCtl.pin), (void *)arg, sizeof(int)))
                return -EINVAL;
            printk("ledPin = %d, mode = OUTPUT\n", ledCtl.pin);
            
            offset = ledCtl.pin/10;
            val = ioread32(&(pRegs->GPFSEL[offset]));
            item = ledCtl.pin % 10;

            val &= ~(0x7 << (item * 3));
            val |= ((1 & 0x7) << (item * 3));
            iowrite32(val, &(pRegs->GPFSEL[offset]));
            break;
        }
        case MY_IOC_ON:
        {
            if(copy_from_user((void *)&ledCtl, (void *)arg, sizeof(LedCtl)))
                return -EINVAL;
            printk("led %d [%s]\n", ledCtl.pin, ledCtl.act);

            offset = ledCtl.pin/32;
            mask = (1 << (ledCtl.pin%32));
            iowrite32((u32)mask, &(pRegs->GPSET[offset]));
            break;
        }
        case MY_IOC_OFF:
        {
            if(copy_from_user((void *)&ledCtl, (void *)arg, sizeof(LedCtl)))
                return -EINVAL;
            printk("led %d [%s]\n", ledCtl.pin, ledCtl.act);

            offset = ledCtl.pin/32;
            mask = (1 << (ledCtl.pin%32));
            iowrite32((u32)mask, &(pRegs->GPCLR[offset]));
            break;
        }
        case MY_IOC_CLEAR:
        {

        }
    }    
    return 0;    
}    
        
struct file_operations simple_fops =    
{    
    .owner           = THIS_MODULE,    
    .read            = simple_read,         
    .write           = simple_write,        
    .unlocked_ioctl  = simple_ioctl,        
    .open            = simple_open,         
    .release         = simple_close,      
};    
        
int simple_init(void)    
{    
    int ret;    
        
    ret = alloc_chrdev_region( &id, 0, 1, DEVICE_NAME );    
    if ( ret ){    
        printk( "alloc_chrdev_region error %d\n", ret );    
        return ret;    
    }    
        
    cdev_init( &cdev, &simple_fops );    
    cdev.owner = THIS_MODULE;    
        
    ret = cdev_add( &cdev, id, 1 );    
    if (ret){    
        printk( "cdev_add error %d\n", ret );    
        unregister_chrdev_region( id, 1 );    
        return ret;    
    }    
        
    class = class_create( THIS_MODULE, DEVICE_NAME );    
    if ( IS_ERR(class)){    
        ret = PTR_ERR( class );    
        printk( "class_create error %d\n", ret );    
        
        cdev_del( &cdev );    
        unregister_chrdev_region( id, 1 );    
        return ret;    
    }    
        
    dev = device_create( class, NULL, id, NULL, DEVICE_NAME );    
    if ( IS_ERR(dev) ){    
        ret = PTR_ERR(dev);    
        printk( "device_create error %d\n", ret );    
        
        class_destroy(class);    
        cdev_del( &cdev );    
        unregister_chrdev_region( id, 1 );    
        return ret;    
    }

    mem = ioremap(BCM2837_GPIO_BASE, BCM2837_GPIO_SIZE);
    pRegs = mem;
    
    return 0;    
}    
        
    
void simple_exit(void)    
{    
    iounmap(mem);
    device_destroy(class, id );    
    class_destroy(class);    
    cdev_del( &cdev );    
    unregister_chrdev_region( id, 1 );    
}    
    
        
module_init(simple_init);    
module_exit(simple_exit);  
    
MODULE_LICENSE("Dual BSD/GPL");

