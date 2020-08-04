
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
//定义pool中申请大块内存的条件是要超过（ngx_pagesize - 1）个字节
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

//nginx_pool的默认初始化内存大小为16K
#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)
//内存池的申请按照16个字节对齐
#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;
    void                 *data;
    ngx_pool_cleanup_t   *next;
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

//大块内存(超过4k-1)存储结构，采用单链表，alloc存储实际申请的内存指针
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    //保存malloc/calloc的申请的内存指针
    void                 *alloc;
};

//小块内存的存储结构，申请连续内存，然后通过移动last的指针来控制可用内存
typedef struct {
    //可用内存的开始地址
    u_char               *last;
    //可用内存的结束地址，实际可用内存实在last和end之间
    u_char               *end;
    //链接下一个nginx_pool
    ngx_pool_t           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;


//nginx的内存池结构
struct ngx_pool_s {
    //初始化时候会申请一块内存，比如申请1024个字节，1024-sizeof(nginx_pool_s)这些内存都会被d记录
    ngx_pool_data_t       d;
    //超过max的个字节，nginx会采用大块内存申请
    size_t                max;
    //保存当前可用pool的指针
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    //大块内存的链表
    ngx_pool_large_t     *large;
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
};


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;


ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
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
