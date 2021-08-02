# Ringbuffer

> 基于5.4.0-73-generic分析
>
> 例子：https://blog.csdn.net/yusiguyuan/article/details/18368457
>
> 内核：https://blog.csdn.net/yusiguyuan/article/details/19906363

# 例子分析

## 相关数据结构

~~~c
struct _ringbuf_ {
    pthread_mutex_t mutex;
    unsigned int buf_size;
    unsigned char * buf_head;
    unsigned char * buf_tail;
    unsigned char buf_start[1];
};
~~~

​	对于环形缓冲区来说，其实主要的就是对于一个已经用过的区域和未用过的区域的判断，其实超简单的。。。。

## 条件变量

​	在该例子中使用了条件变量进行控制

* pthread_cond_init()  初始化默认条件下的条件变量（另一个参数为NULL，否则该条件变量的特性将由参数中的属性值来决定）
* int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex) 函数将解锁mutex参数指向的互斥锁，并使得当前线程阻塞在cv参数指向的条件变量上。

# 内核相关数据结构体

```c
struct ring_buffer {
	unsigned			flags;
	int				cpus;   // ring buffer 中包含的cpu个数
	atomic_t			record_disabled;  // 整个ring buffer的禁用标志,用原子操作了防止竞争,只有当record_disabled的值等于0时,才允许操作RB
	atomic_t			resize_disabled;
	cpumask_var_t			cpumask; // CPU 位图

	struct lock_class_key		*reader_lock_key;

	struct mutex			mutex; // 访问锁

	struct ring_buffer_per_cpu	**buffers; //CPU的缓存区页面,每个CPU对应一项

	struct hlist_node		node;
	u64				(*clock)(void); // RB所用的时间,用来计数时间戳

	struct rb_irq_work		irq_work;
	bool				time_stamp_abs;
};
```

每个cpu的缓存结构体为：

~~~c
struct ring_buffer_per_cpu {
	int				cpu;  // 该cpu_buffer所在的cpu
	atomic_t			record_disabled;
	struct ring_buffer		*buffer; // cpu_buffer 所在的 ring buffer
	raw_spinlock_t			reader_lock;	/* serialize readers */
	arch_spinlock_t			lock;
	struct lock_class_key		lock_key;
	struct buffer_data_page		*free_page;
	unsigned long			nr_pages;
	unsigned int			current_context;
	struct list_head		*pages;
	struct buffer_page		*head_page;	/* read from head */
	struct buffer_page		*tail_page;	/* write to tail */
	struct buffer_page		*commit_page;	/* committed pages */
	struct buffer_page		*reader_page;
	unsigned long			lost_events;
	unsigned long			last_overrun;
	unsigned long			nest;
	local_t				entries_bytes;
	local_t				entries;
	local_t				overrun;
	local_t				commit_overrun;
	local_t				dropped_events;
	local_t				committing;
	local_t				commits;
	local_t				pages_touched;
	local_t				pages_read;
	long				last_pages_touch;
	size_t				shortest_full;
	unsigned long			read;
	unsigned long			read_bytes;
	u64				write_stamp;
	u64				read_stamp;
	/* ring buffer pages to update, > 0 to add, < 0 to remove */
	long				nr_pages_to_update;
	struct list_head		new_pages; /* new pages to add */
	struct work_struct		update_pages_work;
	struct completion		update_done;

	struct rb_irq_work		irq_work;
};
~~~



















