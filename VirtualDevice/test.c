#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>

#define DEMO_NAME "demo_dev"

static dev_t dev;
static struct cdev *demo_cdev = NULL;
static signed count = 1;
static char device_buffer[1024];
#define MAX_DEVICE_BUFFER_SIZE 64

static int demodrv_open(struct inode *inode, struct file *file) {
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);
    printk("%s: major: %d minor: %d\n", __func__, major, minor);
    return 0;
}

static int demodrv_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t demodrv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) { // __user is used to remind buf that it belongs to user space
                                       // file is represents the opened device file
                                       // cout is represents the buf size user want to use
                                       // ppos stands for position pointer
    int actual_readed;
    int max_free;
    int need_read;
    int ret;

    max_free = MAX_DEVICE_BUFFER_SIZE - *ppos; 
    need_read = max_free > count ? count : max_free;
    if (need_read == 0) {
        printk("no space for read\n");
    }
    ret = copy_to_user(buf, device_buffer + *ppos, need_read);
    if (ret == need_read) {
         return 0;
    }
    actual_readed = need_read - ret;
    *ppos += actual_readed; // update the ppos pointer
    printk("%s, actual_readed = %d, pos = %d\n", __func__, actual_readed, *ppos);
    return actual_readed;
}

static ssize_t demodrv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    int actual_write;
    int free;
    int need_write;
    int ret;
    free =  MAX_DEVICE_BUFFER_SIZE - *ppos;
    need_write = free > count  ? count : free;
    if (need_write == 0) {
        printk("no space available for write\n");
    }
    ret = copy_from_user(device_buffer + *ppos, buf , need_write);
    if (ret == need_write) {
        return -EFAULT;
    }
    actual_write = need_write - ret;
    *ppos += actual_write;
    printk("%s: actual_write = %d, ppos = %d\n", __func__, actual_write, *ppos);
    return actual_write;
}

static const struct file_operations demodrv_fops = {
    .owner = THIS_MODULE,
    .open = demodrv_open,
    .release = demodrv_release,
    .read = demodrv_read,
    .write = demodrv_write,
};


//static struct device *mydemodrv_device;

static struct miscdevice mydemodrv_misc_device = { // miscdevice结构体
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEMO_NAME,
    .fops = &demodrv_fops,
};

static int __init simple_char_init(void) {
    int ret;
    ret = alloc_chrdev_region(&dev, 0, count, DEMO_NAME);
    if (ret) {
        printk("failed to allocate char device region\n");
    }
    demo_cdev = cdev_alloc(); // 抽象设备分配
    if (!demo_cdev) {
        printk("cdev_alloc failed\n");
        goto unregister_chrdev;
    } 
    cdev_init(demo_cdev, &demodrv_fops); //主要是对空间起到一个清零作用并较之cdev_alloc多了一个ops的赋值操作。
    ret = cdev_add(demo_cdev, dev, count); 
    if (ret) {
        printk("cdev_add failed\n");
        goto cdev_fail;
    }
    printk("succeeded register char device: %s\n" , DEMO_NAME);
    printk("Major number = %d, minor number = %d\n", MAJOR(dev), MINOR(dev));
    return 0;

cdev_fail:
    cdev_del(demo_cdev);

unregister_chrdev:
    unregister_chrdev_region(dev, count);
    return ret;
}

static void __exit simple_char_exit(void) {
    printk("removing device\n");
    if (demo_cdev) {
        cdev_del(demo_cdev);
    }
    unregister_chrdev_region(demo_cdev, count);
}

module_init(simple_char_init);
module_exit(simple_char_exit);
MODULE_AUTHOR("Clock");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("simple character device");
