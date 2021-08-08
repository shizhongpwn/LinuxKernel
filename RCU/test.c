#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/kthread.h>
#include <linux/delay.h>

struct foo {
    int a;
    struct rcu_head rcu;
};
static struct foo *g_ptr;
static void myrcu_reader_thread(void *data)  {
    struct foo *p = NULL;
    while (1) {
        msleep(200);
        rcu_read_lock(); // 创建临界区
        p = rcu_dereference(g_ptr); // 获取被保护的指针
        if (p) {
            printk("%s: read a = %d\n",__func_, p->a);
        }
        rcu_read_unlock();
    }
}
static void myrcu_del(struct rcu_head *ch) {
    struct foo *p = container_of(rh, struct foo, c);
    printk("%s a= %d\n", __func__, p->a);
    kfree(p);
}

static void myrcu_write_thread(void *p) {
    struct foo *new;
    struct foo *old;
    int value = (unsigned long)p;
    while (1) {
        msleep(400);
        struct foo *new_ptr = kmalloc(sizeof(struct foo), GFP_KERNEL);
        old = g_ptr;
        printk("%s write to new %d\n", __func__, value);
        *new_ptr = *old;
        new_ptr->a = value;
        rcu_asssign_pointer(g_ptr, new_ptr); // 修改原始的g_ptr指向新的数据块
        //synchronize_rcu(); // 保证原有reader对旧数据的访问可以接着进行，而不受影响，因为有的情况下更新之后会kfree原有的数据，这个就是防止对旧数据有引用的reader可以执行完而不受到影响。
        call_rcu(&old->rcu, myrcu_del); // 调用回调函数，删除旧数据。
        value++;
    }
}
