#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <asm/io.h>
#include <asm/uaccess.h>

//#include "cdata_ts_ioctl.h"

#define MSG(arg) printk(KERN_INFO ##arg)

void cdata_ts_handler(int irq, void *priv, struct pt_regs *reg)
{
  printk("HELLO!!");
}


static int cdata_ts_open(struct inode *inode, struct file *filp)
{

    unsigned int reg;

    reg = GPGCON;

    printk(KERN_INFO "GPGCON ADDR: %08x\n", reg);


    set_gpio_ctrl(GPIO_XMON);    
    set_gpio_ctrl(GPIO_XPON);
    set_gpio_ctrl(GPIO_YMON);
    set_gpio_ctrl(GPIO_YPON);

/*
    set_gpio_mode(GPIO_MODE_XMON);
    set_gpio_mode(GPIO_MODE_nXPON);
    set_gpio_mode(GPIO_MODE_YMON);
    set_gpio_mode(GPIO_MODE_nYPON);
*/
    ADCTSC = DOWN_INT | XP_PULL_UP_EN | \
             XP_AIN | XM_HIZ | YP_AIN | YM_GND | \
             XP_PST(WAIT_INT_MODE);

    if (request_irq(IRQ_TC, cdata_ts_handler, 0, 
        "cdata-ts", 0))
        {
        printk(KERN_ALERT "cdata: request irq fail\n");
        return -1;
        }


    return 0;
}

static int cdata_ts_close(struct inode *inode, struct file *filp)
{

    return 0;
}

static int cdata_ts_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{

    return 0;
}


static ssize_t cdata_ts_read(struct file *filp, const char *buf, size_t size, loff_t *off)
{
    
    return 0;
}



static ssize_t cdata_ts_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{
    return 0;
}


static struct file_operations cdata_ts_fops =
{
    owner: THIS_MODULE, //讓kernel自己去控制INC / DEC
    open: cdata_ts_open, //open(), close(), 都會使用struct inode and struct file
    release: cdata_ts_close, //struct file的指標通常用 *filp
    ioctl: cdata_ts_ioctl, //ioctl()需要四個參數:
    //inode & file & unsigned int cmd & unsigned long arg （address)
    read: cdata_ts_read,
    write: cdata_ts_write,

};

static struct miscdevice cdata_ts_misc = {

    minor:  CDATA_TS_MINOR,
    name:   "cdata-ts",
    fops:   &cdata_ts_fops,
};

static int cdata_ts_module_init(void)
{

    if(misc_register(&cdata_ts_misc) !=0 )
        {
        MSG("cannot open\n");
        return -1;
        }
    return 0;
}

static int cdata_ts_module_exit(void)
{
    misc_deregister(&cdata_ts_misc);
    return 0;
}

module_init(cdata_ts_module_init);
module_exit(cdata_ts_module_exit);

MODULE_LICENSE("GPL");
