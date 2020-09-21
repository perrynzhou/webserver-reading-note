## nginx编译选项说明

- 下载nginx源代码
```
$ wget http://nginx.org/download/nginx-1.18.0.tar.gz
$ tar zxvf nginx-1.18.0.tar.gz && cd nginx-1.18.0
$ configure 后面可以跟着很多编译选项
```

- configure 编译选项说明
  - --prefix=path,制定nginx的安装目录
  - --wth-stream,启动stream模块，让nginx能够工作在4层网络，直接处理TCP/UDP协议
  - --with-threads,启用nginx线程池子，让nginx使用线程池来处理数据，“卸载”阻塞操作
  - --with-pcre=path,--with-openssl=path,虽然linux内置了这些库，但是有时候版本太低会导致bug.所以这2个编译选项能够制定pcre和openssl源代码，从而获取最新版本
  - --with-{xxx-moudle},--without-{xxx-module},用这些配置来决定启用和禁用哪些自带的功能模块
  - --build=name，这可以在版本信息中制定公司名称。构建日期，源代码版本号等信息
    ```
    ./configure --build="${USER} build at `date +%Y%m%d`"
    ```
  - --with-debug,启用nginx的调试模式
  - --with-ld-opt=options,这个选项指定编译链接时候的额外参数，可以链接其他第三方库，例如使用hemalloc
    ```
    ./configure -with-ld-opt="-ljemmalloc"
    ```
  - --add-module=path,指定nginx第三方模块的源代码，这样nginx可以把第三方模块像自带模块一样进行编译，这个选项可以在configure后面出现任意多次，从而达到一次为nginx添加任意多的第三方模块
  - --add-dynamic-module=patj,这个功能和--add-module功能相同，--add-dynamic-module分离nginx主可执行程序和模块，可以在启动时候灵活组合动态加载，方便、nginx更新，使用"make modules"只编译变动模块，而nginx核心代码无须重新编译

- nginx配置
  - 进程管理相关配置(全局域配置)
    - worker_processes {number}|auto,设置nginx能够启动的worker的进程数目，通常nginx的worker进程和CPU核心数相等时能获取最佳的性能，这个配置一般需要和worker_cpu_affinity指令配合使用
    - worker_cpu_affinity auto | {cpumask},指定worker进程运行在某个cpu核心上。
    ```
    //配置2个？nginx工作进程
    worker_processes 2 
    //这2个进程分别绑定在0和1核心的CPU上
    worker_cpu_affinity 0001 00010
    ```
    - master_process on|of,决定使用启用nginx进程池，默认是on,如果设置为off,nginx启动不会建立master进程，只会启动一个worker进程来处理哦请求.
    - daemon on|off,决定是否启用daemon方式运行nginx,默认是on,如果设置为off方便我们调试
    - working_directory {path},配置nginx工作目录，仅仅存放nginx的coredump文件
    - working_shutdown_timeout {time},当使用"nginx -s quit"要求终止运行,nginx将等待的时间，然后强制关闭进程，这个选项可以很好解决系统里面出现大量”is shutting down"状态的nginx进程问题