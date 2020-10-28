## web服务器源码 阅读笔记

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/05/24 |


## 目标

- 梳理nginx-1.19.2/openresty-1.17.8.2/tengine-2.3.2功能点
- 整理清楚nginx高效的原因本质
- 针对该版本在代码层面梳理一份有C语言基础的同学都可以读懂

## tcp/ip

### tcp/ip协议
- [tcp协议初探](./document/tcp-ip/tcp协议初探.md)
- [tcp连接和关闭](./document/tcp-ip/tcp连接和关闭.md)
- [tcp选项和路径MTU](./document/tcp-ip/tcp选项和路径MTU.md)
- [tcp状态图](./document/tcp-ip/tcp状态转换图.md)
- [tcp超时和重传](./document/tcp-ip/tcp超时和重传.md)
- [TCP三次握手和四次挥手简单分析](./document/tcp-ip/TCP三次握手和四次挥手简单分析.md)


### tcp/ip编程
- [udp编程初探](./document/tcp-ip/udp编程初探.md)
- [tcp是一种流式协议](./document/tcp-ip/tcp是一种流式协议.md)
## nginx源码
  - [常用web服务器介绍](./document/nginx-1.19.2/常用web服务器介绍.md)
  - [nginx内存池实现和分析](./document/nginx-1.19.2/nginx内存池实现和分析.md)
  - [nginx模块开发](./document/nginx-1.19.2/nginx模块开发.md)
  - [nginx源码调试](./document/nginx-1.19.2/nginx源码调试.md)
  - [nginx服务器架构探讨](./document/nginx-1.19.2/nginx服务器架构探讨.md)
  - [nginx缓存管理进程](./document/nginx-1.19.2/nginx缓存管理.md)
  - [nginx缓存加载进程](./document/nginx-1.19.2/缓存加载进程.md)
  - [nginx编译和配置](./document/nginx-1.19.2/nginx编译选项和配置.md)
  - [nginx模块介绍](./document/nginx-1.19.2/nginx模块概述.md)
  - [事件驱动模型](./document/nginx-1.19.2/事件驱动模型.md)
  - [module定义和说明](./document/nginx-1.19.2/module定义和说明.md)
  - [http和tcp-udp处理状态阶段说明](./document/nginx-1.19.2/http和tcp-udp处理状态阶段说明.md)  
  - [http处理全过程追踪](./document/nginx-1.19.2/http处理全过程追踪.md)
  - [upstream处理逻辑ngx_http_proxy_handler函数说明](https://github.com/perrynzhou/webserver-note/tree/perryn/dev/nginx-1.19.2/src/http/modules/ngx_http_proxy_module.c#L849)
  - [nginx请求处理前需要做哪些事情?](./document/nginx-1.19.2/nginx请求处理前需要做哪些事情.md)

## openresty
  - [OpenResty基本介绍](./document/openresty-1.17.8.2/OpenResty基本介绍.md)
  - [OpenResty源码编译](./document/openresty-1.17.8.2/OpenResty源码编译.md)


## tengine