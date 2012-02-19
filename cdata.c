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

#define BUF_SIZE 128
#define LCD_SIZE 320*240

//unsigned long *fb;   //直接這樣寫也不對會有re-entrance, 所以要用filp->private_data
//改成下面這個struct來實做

struct cdata_t {
   unsigned long *fb;
   unsigned char *buf;
   unsigned int index;
   unsigned int offset;

   struct timer_list flush_timer;
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
    cdata->buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    cdata->index = 0;
    cdata->offset = 0;
    cdata->fb = ioremap(0x33F00000, 320*240*4);
    init_timer(&cdata->flush_timer);

    filp->private_data = (void *)cdata;


    return 0;
}

static ssize_t cdata_read(struct file *filp, const char *buf, size_t size,
loff_t *off)
{

}


//static flush_lcd(void * priv)
static flush_lcd(unsigned long priv)
{
    struct cdata_t *cdata = (struct cdata_t *)priv;
    unsigned char *linebuf;
    unsigned int index, i;
    unsigned char *fb;
    unsigned int offset;

    linebuf = cdata->buf;
    index = cdata->index;
    fb = (unsigned char *)cdata->fb;
    offset = cdata->offset;

    for (i =0; i<index; i++)
     {
        writeb(linebuf[i],fb+offset);
        offset++;
        if (offset >= LCD_SIZE*4)
          {
            printk(KERN_INFO "BANGO: %d\n", offset);
            offset =0;
          }
     }

    //printk(KERN_INFO "offset: %d\n", offset);

    cdata->index =0;
    cdata->offset = offset;

}

static void cdata_wake_up(unsigned long priv)
{
    //Wakeup function for deferred task, let process state becomes "ready" from TASK_INTERRUTIBLE state. If you prepare to do process scheduling, then the task will wait in the running queue


}


static ssize_t cdata_write(struct file *filp, const char *buf, size_t size,
loff_t *off)
{
    int i=0;

    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
    unsigned char *linebuf;
    unsigned int index;
    struct timer_list *timer;

    //lock
    index = cdata->index;
    linebuf = cdata->buf;
    timer = &cdata->flush_timer;
    //unlock

    for (i=0; i < size; i++)
     {
       if (index >= BUF_SIZE)
        {
             //開始要解決花費很多時間的問題.

           cdata->index = index;
           //FIXME: kernel scheduleing, deferred it, using Kernel timer
           //flush_lcd((void *)cdata);  //花費很多時間
           timer->expires = jiffies + 1*HZ;  //HZ: CPU跑一秒的意思
           timer->data = (unsigned long)cdata;
           timer->function = flush_lcd;
           add_timer(timer);  //加入kernel. 開始計數


           // FIXME: process scheduling, 做完process schedule後切回來的時候才需要讀回index
	    current->state = TASK_INTERRUPTIBLE;
	    schedule();

           index = cdata->index;



        }

  	copy_from_user(&linebuf[index], &buf[i], 1);
	//printk(KERN_INFO "index: %d ; pix: %d\n", index, linebuf[index]);
       index ++;
     }

    cdata->index = index;

    return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;

    flush_lcd((void *) cdata);

    del_timer(&cdata->flush_timer);
    kfree(cdata->buf);
    kfree(cdata);
    return 0;
}

static int cdata_flush(struct file *filp)
{
    printk(KERN_INFO "Flush\n");
}

static int cdata_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{

    int n;
    unsigned long size;
    unsigned long *fb;
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;

    switch (cmd)
     {
        case IOCTL_CLEAR:
             
              //size = *((int *)arg); //FIXME, cannot access the user space 
              copy_from_user(&size, &arg, 1);
              printk(KERN_INFO "IOCTL-KERN size: %d; arg: %d\n", size, arg);

               // Lock, 因為有可能Apps會有folk process同時使用這各資料結構, 所以需要lock 機制
              fb = cdata->fb;
               // unlock


              for (n=0; n< size ; n++)
                 {
                   writel(0x00ff00ff, fb++);
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
