## OpenResty源码编译

| 作者 | 时间 |QQ技术交流群 |
| ------ | ------ |------ |
| perrynzhou@gmail.com |2020/09/17 |中国开源存储技术交流群(672152841) |

- 安装nginx依赖库
```
# yum install gcc pcre pcre-devel zlib zlib-devel openssl openssl-devel -y
```
- 下载源码包
```
# wget https://openresty.org/download/openresty-1.17.8.2.tar.gz
# tar zxvf openresty-1.17.8.2.tar.gz 
```
- 编译安装
```
# cd openresty-1.17.8.2
# CFLAGS="-g -O0" ./configure
# make -j32 && make install
```

- 安装测试
  - 创建demo.conf配置文件
	```
	worker_processes 1;
	events {
	 worker_connection 512;
	}
	http {
		server {
			listen 7000;
			server_name *.*;
			location / {
				#content_by_lua_block是一个专用指令，可以在配置文件中写入lua代码，产生响应内容，指令里面调用了ngx.print接口
				content_by_lua_block {
				ngx.print("hello,first demo for openresty\n")
				}
			}
		}
	}
	```
	- 启动openresty
	```
	# /usr/local/openresty/bin/openresty  -c hello.conf 
	# ps -ef|grep nginx
root     184109      1  0 08:05 ?        00:00:00 nginx: master process /usr/local/openresty/bin/openresty -c hello.conf
nobody   184113 184109  0 08:05 ?        00:00:00 nginx: worker process
root     185893 162740  0 08:07 pts/1    00:00:00 grep --color=auto nginx
	
	```
	- 客户端验证
	```
	# curl 127.0.0.1:7000                          
hello,first demo for openresty
	```
- openresty基本命令
```
//基本和nginx命令完全兼容
# /usr/local/openresty/bin/openresty  -h
nginx version: openresty/1.17.8.2
Usage: nginx [-?hvVtTq] [-s signal] [-c filename] [-p prefix] [-g directives]

Options:
  -?,-h         : this help
  -v            : show version and exit
  -V            : show version and configure options then exit
  -t            : test configuration and exit
  -T            : test configuration, dump it and exit
  -q            : suppress non-error messages during configuration testing
  -s signal     : send signal to a master process: stop, quit, reopen, reload
  -p prefix     : set prefix path (default: /usr/local/openresty/nginx/)
  -c filename   : set configuration file (default: conf/nginx.conf)
  -g directives : set global directives out of configuration file
```
- openresty配置指令
  - openresty里面ngx_lua和stream_lua分别属于不同子系统，ngx_lua是http的子系统；stream_lua是处理tcp/udp子系统。
  - openresty里面中最基本的三个指令分别是lua_package_path、lua_package_cpath、lua_code_cache。lua_package_path和lua_package_cpath提供以字符串形式确定lua库和so库的查找路径，文件名称使用"?"作为通配符，多个路径使用";"分隔默认查找路径是";;".lua_code_cache是在openresty中启用lua代码缓存的功能，源代码文件里lua代码被LuaVM加载后就被缓存起来，如果lua_code_cache为off,则每次从磁盘进行加载lua库，这种方式方便调试
  ```
  lua_package_path  "$prefix/service/?.lua;;";
  lua_package_cpath "$prefix/service/lib/?.so;;";
  lua_code_cache on;
  //或者
  lua_code_cache off;
  ```
