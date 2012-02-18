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

#include "cdata_ioctl.h"


//unsigned long *fb;   //直接這樣寫也不對會有re-entrance, 所以要用filp->private_data
//改成下面這個struct來實做

struct cdata_t {
   unsigned long *fb;
};


static int cdata_open(struct inode *inode, struct file *filp)
{
    int minor, major;
    struct cdata_t *cdata;


    printk(KERN_INFO "CDATA: in open\n");

    minor = MINOR(inode->i_rdev);
    major = MAJOR(inode->i_rdev);
    printk(KERN_INFO "CDATA Major no: %d & Minor no: %d\n", major, minor);


    cdata = kmalloc(sizeof(struct cdata_t), GFP_KERNEL);
    cdata->fb = ioremap(0x33F00000, 320*240*4);
    filp->private_data = (void *)cdata;


    return 0;
}

static ssize_t cdata_read(struct file *filp, const char *buf, size_t size,
loff_t *off)
{

}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size,
loff_t *off)
{
    int i=0;

/*
    for (i=0; i<500000; i++)
     {
         ;
         //schedule();
     }
*/

/*
    printk(KERN_INFO "WRITE\n");
    while (1)
     {
        printk(KERN_INFO "whiling\n");
        //current->state = TASK_UNINTERRUPTIBLE;
        schedule();
     }
*/

/*
    unsigned long *fb;
    
    fb = ioremap(0x33f00000, 320*240*4); 
    for (i=0; i< 320*240; i++)
       writel(0xffff00, fb+i);

    printk(KERN_INFO "WRITE FB\n");
*/


    return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
    return 0;
}

static int cdata_flush(struct file *filp)
{
    printk(KERN_INFO "Flush\n");
}

static int cdata_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{

    int n, size;
    unsigned long *fb;
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;

    switch (cmd)
     {
        case IOCTL_CLEAR:
             
              size = *((int *)arg); //FIXME, cannot access the user space 
              printk(KERN_INFO "IOCTL-KERN\n");

               // Lock, 因為有可能Apps會有folk process同時使用這各資料結構, 所以需要lock 機制
              fb = cdata->fb;
               // unlock


              for (n=0; n< size ; n++)
                 {
                   writel(0x00ffffff, fb++);
                 }
        break;
     }

}

static struct file_operations cdata_fops = {
    owner: THIS_MODULE,
    open: cdata_open,
    release: cdata_close,
    read: cdata_read,    
    write: cdata_write,
    ioctl: cdata_ioctl,
    flush: cdata_flush,
};

static int cdata_init_module(void)
{
    if (register_chrdev(121, "cdata", &cdata_fops) < 0) {
        printk(KERN_INFO "CDATA: can't register driver\n");
    return -1;
     }

    return 0;
}

static void cdata_cleanup_module(void)
{
    unregister_chrdev(121, "cdata");
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
