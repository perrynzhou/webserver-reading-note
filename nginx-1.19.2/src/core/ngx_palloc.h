
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
//设置nginx的从内存池分配内存的最大内存申请量
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)
//nginx内存池的初始化内存申请量
#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)
//内存申请的对齐方式
#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)

//内存池中已经申请内存的数据块的回调函数
typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;
    void                 *data;
    ngx_pool_cleanup_t   *next;
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;
//nginx大块内存申请结构体
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    //大块内存的地址
    void                 *alloc;
};

//nginx非大块内存结构的定义
typedef struct {
    //可用内存起始地址
    u_char               *last;
    //申请内存的结束地址
    u_char               *end;
    //下一个内存池的地址
    ngx_pool_t           *next;
    //申请内存失败的次数
    ngx_uint_t            failed;
} ngx_pool_data_t;

//nginx内存池的定义
struct ngx_pool_s {
    //当前内存池中的小块内存的地址
    ngx_pool_data_t       d;
    size_t                max;
    //当前内存池的指针
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    //大块内存的结构链表
    ngx_pool_large_t     *large;
    ngx_pool_cleanup_t   *cleanup;
    //日志的结构
    ngx_log_t            *log;
};


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;

//内存池的创建
ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
//内存池的销毁
void ngx_destroy_pool(ngx_pool_t *pool);
//重置内存池为初始化状态
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
