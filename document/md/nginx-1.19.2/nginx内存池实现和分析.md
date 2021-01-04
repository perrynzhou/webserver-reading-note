
## nginx内存池实现和分析

| 作者 | 时间 |QQ技术交流群 |
| ------ | ------ |------ |
| perrynzhou@gmail.com |2020/06/23 |中国开源存储技术交流群(672152841) |


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
    //可用内存的末尾地址
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

- ngx_create_pool:nginx的内存池创建
- ngx_destroy_pool:销毁一个内存池
- ngx_reset_pool:重置一个内存池中的小块内存，同时释放大块内存
- ngx_palloc:通过系统调用函数申请内存
- ngx_pnalloc:从内存池中申请内存
- ngx_pmemalign:按照对齐方式方式申请large结构体内存
- ngx_pfree:释放大块内存


### 内存池函数注释
[ngx_palloc.h](https://github.com/perrynzhou/nginx-1.19.1-reading-note/blob/perryn/dev/nginx-1.19.1/src/core/ngx_palloc.h)
[ngx_palloc.c](https://github.com/perrynzhou/nginx-1.19.1-reading-note/blob/perryn/dev/nginx-1.19.1/src/core/ngx_palloc.c)