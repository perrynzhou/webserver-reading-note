
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    //存储申请内存的开始地址
    void        *elts;
    //当前array中元素的个数
    ngx_uint_t   nelts;
    //申请元素类型的大小
    size_t       size;
    //申请内存的次数
    ngx_uint_t   nalloc;
    //分配内存的内存池
    ngx_pool_t  *pool;
} ngx_array_t;

//初始化一个ngx_array
ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
//销毁一个ngx_array，实际就是把a->pool中的指针重置
void ngx_array_destroy(ngx_array_t *a);
//从array中插入一个数据，同时返回数据的地址
void *ngx_array_push(ngx_array_t *a);
//从array中连续申请一块内存，每个内存大小为a->size,申请n个
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);


static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
