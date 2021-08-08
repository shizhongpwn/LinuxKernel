# FileSystem

> 文件系统算是kernel里面稍微简单一点的了，因此可以先实现一个小的文件系统，方便理解其设计思想。
>
> 参考：https://www.jianshu.com/p/8966d121263b

## 总体设计

~~~c
|Dummy Block|Super Block|IMap|BMap|Inode Table|Data blocks|
~~~

​	每个块的大小4096bytes，每个Inode含有十个块，因此单个文件的最大大小为40KB，支持的最小磁盘大小为24k。

### Super Block

~~~c
struct HUST_fs_super_block {
    uint64_t version;
    uint64_t magic;
    uint64_t block_size;
    uint64_t inodes_count;
    uint64_t free_blocks;
    uint64_t blocks_count;
    uint64_t bmap_block;
    uint64_t imap_block;
    uint64_t inode_table_block;
    uint64_t data_block_number;
    char padding[4016];
};
~~~

* magic : 1314522
* incodes_count 文件系统支持的inode个数
* blocksize 

### HUST_inode结构

~~~c
struct HUST_inode {  
    mode_t mode; //sizeof(mode_t) is 4  
    uint64_t inode_no;  
    uint64_t blocks;  
    uint64_t block[HUST_N_BLOCKS];   // HUST_N_BLOCKS 宏定义为10，固定inode的大小
    union {  
        uint64_t file_size;  
        uint64_t dir_children_count;  
    };  
    int32_t i_uid;   
    int32_t i_gid;  
    int32_t i_nlink;  
    int64_t i_atime;  
    int64_t i_mtime;  
    int64_t i_ctime;  
    char padding[112];  
};  
~~~

​	对应磁盘上面的inode结构。

* Mode 代表文件还是目录
* Blocks 代表inode的大小
* i_uid和i_gid等用来方便多用户管理
* padding是为了对齐
* block用来索引块的位置

### 文件的目录系统结构

~~~c
struct HUST_dir_record {  
    char filename[HUST_FILENAME_MAX_LEN];  
    uint64_t inode_no;  
};
~~~

文件记录是为了存储目录项。

* HUST_FILENAME_MAX_LEN定义为256,所以文件名的最大长度为256



## 文件系统的实现

### 格式化文件

​	首先要创建一个符合磁盘布局的映像文件，因此需要实现一个格式化程序，项目中的mkfs就是，主要功能：

1. 超级块的创建
2. 计算出imap和bmap的大小，创建imap和bmap相关的内存区域
3. 计算出inode_table_block的大小
4. 写入bmap， imap, inode_table_block.

### 文件系统的加载与删除

文件系统基本结构体`files_system_type` 

~~~c
struct file_system_type HUST_fs_type = {  
    .owner = THIS_MODULE,  
    .name = "HUST_fs",  
    .mount = HUST_fs_mount,   // mount时启动的程序
    .kill_sb = HUST_fs_kill_superblock, /* unmount */  
};  
~~~

**文件系统本质上是块驱动设备。**

~~~c
/* Called when the module is loaded. */  
int HUST_fs_init(void)  
{  
    int ret;  
    ret = register_filesystem(&HUST_fs_type);  
    if (ret == 0)  
        printk(KERN_INFO "Sucessfully registered HUST_fs\n");  
    else  
        printk(KERN_ERR "Failed to register HUST_fs. Error: [%d]\n", ret);  
    return ret;  
}  
  
/* Called when the module is unloaded. */  
void HUST_fs_exit(void)  

    int ret;  
    ret = unregister_filesystem(&HUST_fs_type);  
    if (ret == 0)  
        printk(KERN_INFO "Sucessfully unregistered HUST_fs\n");  
    else  
        printk(KERN_ERR "Failed to unregister HUST_fs. Error: [%d]\n", ret);  
}  
module_init(HUST_fs_init);  
module_exit(HUST_fs_exit);  
  
MODULE_LICENSE("MIT");  
MODULE_AUTHOR("cv");  
~~~

剩下的都是可以看代码直接看它怎么实现的文件系统的管理方式。



















