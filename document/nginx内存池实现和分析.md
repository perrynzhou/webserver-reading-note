
## nginx内存池实现和分析
### 背景

- 目的是分析nginx的内存池模块，了解内存池相关知识
- nginx为什么要这样设计内存池

### 基本数据结构介绍

- ngx_pool_s:nginx的内存池结构，定义如下
```
struct ngx_pool_s {
    //d:保存可用内存地址信息
    ngx_pool_data_t       d;
    //max:申请大块的的基本条件是申请大小超过max
    size_t                max;
    //current：指向当前内存池结构，初始化时候指向它自己
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    //large:表示大块内存，通过单向链表表示
    ngx_pool_large_t     *large;
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
};
```
- ngx_pool_data_t：存储内存小于(4k-1)的内存块，其结构定义如下

```
typedef struct {
    //可用内存的开始地址
    u_char               *last;
    //内存的末尾地址，扩展一个nginx_pool_t时候指向，这个连续内存的末尾地址
    u_char               *end;
    //指向下一个内存pool
    ngx_pool_t           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;
```



- ngx_pool_large_s：存储内存超过(4k-1)字节的内存块,其结构定义如下
  
```
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    void                 *alloc;
};

```
### 基本函数介绍

- ngx_create_pool:nginx
- ngx_destroy_pool:
- ngx_reset_pool:
- ngx_palloc:
- ngx_pnalloc:
- ngx_pmemalign:
- ngx_pfree: