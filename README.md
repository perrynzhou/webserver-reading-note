## web服务器源码 阅读笔记

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/05/24 |


## 目标

- 梳理nginx-1.19.2/openresty-1.17.8.2/tengine-2.3.2功能点
- 整理清楚nginx高效的原因本质
- 针对该版本在代码层面梳理一份有C语言基础的同学都可以读懂


## nginx源码
- 进程架构
  - [1.常用web服务器介绍](./document/nginx-1.19.2/常用web服务器介绍.md)
  - [2.nginx内存池实现和分析](./document/nginx-1.19.2/nginx内存池实现和分析.md)
  - [3.nginx模块开发](./document/nginx-1.19.2/nginx模块开发.md)
  - [4.nginx源码调试](./document/nginx-1.19.2/nginx源码调试.md)
  - [5.nginx进程模型](./document/nginx-1.19.2/nginx进程模型.md)
  - [6.nginx缓存管理进程](./document/nginx-1.19.2/nginx缓存管理.md)
  - [7.nginx缓存加载进程](./document/nginx-1.19.2/缓存加载进程.md)
  - [8.nginx编译和配置](./document/nginx-1.19.2/nginx编译选项和配置.md)
  - [9.nginx模块介绍](./document/nginx-1.19.2/nginx模块概述.md)
- 模块原理和分析
  - [1.ngx_http_static_module原理和分析（待更新）](./document/nginx-1.19.2/ngx_http_static_module原理和分析.md)
  - [1.ngx_core_module做了什么](./document/nginx-1.19.2/ngx_core_module做了什么.md)

## openresty



## tengine