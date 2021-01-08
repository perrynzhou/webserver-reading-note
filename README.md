## web服务器源码 阅读笔记

| 作者 | 时间 |QQ技术交流群 |
| ------ | ------ |------ |
| perrynzhou@gmail.com |2020/12/05 |中国开源存储技术交流群(672152841) ||


## 目标

- 梳理nginx-1.19.2/openresty-1.17.8.2/tengine-2.3.2功能点
- 整理清楚nginx高效的原因本质
- 针对该版本在代码层面梳理一份有C语言基础的同学都可以读懂

## http
- [HTTP协议介绍](./document/md/md/http/HTTP协议介绍.md)
- [HTTP2协议基础](./document/md/md/http/http2协议基础.md)


##  nginx分析
  - [常用web服务器介绍](./document/md/nginx-1.19.2/常用web服务器介绍.md)
  - [nginx内存池实现和分析](./document/md/nginx-1.19.2/nginx内存池实现和分析.md)
  - [nginx模块开发](./document/md/nginx-1.19.2/nginx模块开发.md)
  - [nginx源码调试](./document/md/nginx-1.19.2/nginx源码调试.md)
  - [nginx服务器架构探讨](./document/md/nginx-1.19.2/nginx服务器架构探讨.md)

  - [nginx-进程架构](./document/md/nginx-1.19.2/nginx-进程架构.md)

  - [nginx缓存管理进程](./document/md/nginx-1.19.2/nginx缓存管理.md)
  - [nginx缓存加载进程](./document/md/nginx-1.19.2/缓存加载进程.md)
  - [nginx编译和配置](./document/md/nginx-1.19.2/nginx编译选项和配置.md)
  - [nginx模块介绍](./document/md/nginx-1.19.2/nginx模块概述.md)
  - [事件驱动模型](./document/md/nginx-1.19.2/事件驱动模型.md)
  - [module定义和说明](./document/md/nginx-1.19.2/module定义和说明.md)
  - [http和tcp-udp处理状态阶段说明](./document/md/nginx-1.19.2/http和tcp-udp处理状态阶段说明.md)  
  - [http处理全过程追踪](./document/md/nginx-1.19.2/http处理全过程追踪.md)
  - [upstream处理逻辑ngx_http_proxy_handler函数说明](https://github.com/perrynzhou/webserver-note/tree/perryn/dev/nginx-1.19.2/src/http/modules/ngx_http_proxy_module.c#L849)
  - [nginx请求处理前需要做哪些事情?](./document/md/nginx-1.19.2/nginx请求处理前需要做哪些事情.md)
  - [nginx响应处理简单分析](./document/md/nginx-1.19.2/nginx响应处理处理简单分析.md)
  - [ngx-hello-world模块编译和测试](./document/md/nginx-1.19.2/ngx-hello-world模块编译和测试.md)
  - [nginx中tcp反向代理.md](./document/md/nginx-1.19.2/2020-11-03-nginx中tcp反向代理.md)
  - [nginx框架功能介绍.md（持续更新）](./document/md/nginx-1.19.2/2020-11-03-nginx框架功能介绍.md)
  - [子请求介绍](./document/md/nginx-1.19.2/2020-11-20-nginx子请求介绍.md)
  - [负载均衡简单介绍](./document/md/nginx-1.19.2/2020-12-22-nginx负载均衡介绍.md)
  - [深入分析ip_hash负载均衡-持续更新](./document/md/nginx-1.19.2/2020-12-23-深入分析ip_hash负载均衡.md)




## openresty
  - [OpenResty基本介绍](./document/md/openresty-1.17.8.2/OpenResty基本介绍.md)
  - [OpenResty源码编译](./document/md/openresty-1.17.8.2/OpenResty源码编译.md)
 - [init_by_lua指令说明.md](./document/md/openresty-1.17.8.2/2020-11-17-init_by_lua指令说明.md)
