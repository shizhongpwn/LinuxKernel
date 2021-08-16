# VirtualDevice

> 这里仅仅记录一些要点，细节可以直接看代码

## 进程状态

进程声明周期的五种状态：

* TASK_RUNNING       可运行态或者就绪态 
* TASK_INTERRUPTIBLE     可中断睡眠态
* TASK_UNINTERRUPTIBLE     不可中断睡眠态
* __TASK_STOPPED         终止态
* EXIT_ZOMBIE      僵尸态

## IO阻塞

​	在进行阻塞IO读取的时候，要首先把进程设置为TASK_INTERRUPTIBLE或者TASK_UNINTERRUPTIBLE的睡眠状态，然后等待设备向缓冲区提供数据，然后将进程唤醒回TASK_RUNNING状态。

​	在Linux内核中，采用`等待队列`的方式来实现进程的阻塞

~~~c
#include<linux/wait.h>
struct __wait_queue_head { // kernel 等待队列数据结构
  spinlock_t lock;
  struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

DECLARE_WAIT_QUEUE_HEAD(name) //静态定义并初始化一个队列

//动态初始化
wait_queue_head_t my_queue;
init_waitqueue_head(&my_queue);

//等待队列元素
struct __wait_queue {
  unsigned int flags;
  void *private;
  wait_queue_func_t func;
  struct list_head task_list;
}
typedef struct __wait_queue wait_queue_t;
~~~

​	同时内核提供了简单的进程睡眠和唤醒的操作：









