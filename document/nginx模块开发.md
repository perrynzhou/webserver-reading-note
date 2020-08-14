### nginx模块开发指南

### nginx模块种类
#### 按照nginx中承担的作用的重要程度
- 核心模块：提供nginx中最基本的功能，网络管理，文件管理，内存管理，配置解析，模块加载等
- 标准模块：nginx编译时候无需指明编译的重要模块，其提供了用于实现http web服务的基本功能，包括反向代理/代理,URL重写，GZIP等功能
- 可选http模块：nginx自带，编译需要指明的模块
- 第三方模块：不属于nginx自带，是由其他第三方开发人员开发

#### 按照nginx源码实现分类

- core模块：负责nginx服务器的核心功能
- http模块:负责nginx服务器的http web服务功能
- event模块：负责nginx 服务器中事件处理服务
- mail模块：负责nginx服务器的邮件服务

### nginx模块开发步骤

- ngx_module_t 结构体定义和配置变量的解析实现
- 模块上下文定义
- nginx_command_t 结构体定义以及命令处理函数实现

