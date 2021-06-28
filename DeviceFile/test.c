#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/ide.h>
#include <linux/delay.h>
#include <linux/cdev.h>
 
#define CHRDEV_MAJOR 200
#define CHRDEV_NAME "char_test"
 
dev_t devno;
struct class *class;
struct device *device;
struct cdev *cdev_t;
static char readbuf[100];
static char writebuf[100];
static char kerneldata[] = {"kernel data!"};
 
#define DEVICE_NUMBER 1
 
static int chartest_open (struct inode *inode, struct file *file)
{
    printk("chartest_open\n");
    return 0;
}
 
static ssize_t chartest_read (struct file *file, char __user *buf,
            size_t cnt, loff_t *offt)
{
    int value = 0;
 
    memcpy(readbuf, kerneldata, sizeof(kerneldata));
    value = copy_to_user(buf, readbuf, cnt);
    if(!value) {
        printk("kernel send data : %s\n", buf);
    } else {
        printk("kernel send data failed %d\n", value);
    }
 
    return 0;
}
 
static ssize_t chartest_write (struct file *file, const char __user *buf,
            size_t cnt, loff_t *offt)
{
    int value = 0;
 
    value = copy_from_user(writebuf, buf, cnt);
    if(!value) {
        printk("kernel recieve data : %s\n", writebuf);
    } else {
        printk("kernel send data failed %d\n", value);
    }
 
    return 0;
}
 
static int chartest_release (struct inode *inode, struct file *filp)
{
    printk("chartest_release\n");
    return 0;
}
 
static struct file_operations chartest_fops = {
    .owner = THIS_MODULE,
    .open = chartest_open,
    .read = chartest_read,
    .write = chartest_write,
    .release = chartest_release,
};
 
static int chartest_init(void)
{
    int ret = 0;
 
    if (CHRDEV_MAJOR) {
        devno = MKDEV(CHRDEV_MAJOR, 0);
        ret = register_chrdev_region(devno, DEVICE_NUMBER, CHRDEV_NAME);
        if (ret < 0) {
            printk("register_chrdev_region failed\n");
            return 0;
        }
    } else {
        ret = alloc_chrdev_region(&devno, 0, DEVICE_NUMBER, CHRDEV_NAME);
        if (ret < 0) {
            printk("alloc_chrdev_region failed\n");
            return 0;
        }
 
    }
 
    cdev_t = kzalloc(sizeof(struct cdev), GFP_KERNEL);
 
    cdev_init(cdev_t, &chartest_fops);
    cdev_t->owner = THIS_MODULE;
 
    cdev_add(cdev_t, devno, 1);
 
    class = class_create(THIS_MODULE, CHRDEV_NAME);
 
    device = device_create(class, NULL, devno, NULL, CHRDEV_NAME);
 
    printk("chartest_init !\n");
 
    return 0;
}
 
static void chartest_exit(void)
{
    unregister_chrdev_region(devno, DEVICE_NUMBER);
    cdev_del(cdev_t);
 
    device_destroy(class, devno);
    class_destroy(class);
 
    printk("chartest_exit\n");
}
 
module_init(chartest_init);
module_exit(chartest_exit);
 
MODULE_LICENSE("GPL");
