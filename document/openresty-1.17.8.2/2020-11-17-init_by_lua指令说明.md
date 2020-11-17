## OpenResty init_by_lua指令说明


| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/11/11 |

##### lua可使用的主要阶段

- init_by_lua/init_by_lua_file指令，处于loading-config阶段，在http块中使用，在nginx管理进程加载配置时候执行，通常用于初始化全局变量或者预加载lua模块
- init_worker_by_lua/init_worker_by_lua_file指令,处于start-worker阶段，在http块中配置，每个nginx进程启动调用的计时器，如果管理进程不允许则只会在init_by_lua中调用，通常用于定期拉取配置/数据或者进行后端服务检查
- set_by_lua/set_by_lua_file指令,处于rewrite阶段，在server、server if、location、location if配置块中使用，设置nginx变量，可以实现复杂的赋值逻辑；此处的阻塞的，lua的代码要做到非常快。
- rewrite_by_lua/rewrite_by_lua_file指令，处于rewrite tail阶段，在http 、server、location、location if配置块中使用，rewrite阶段的处理，可以实现复杂的转发和重定向逻辑
- access_by_lua/access_by_lua_file指令,处于access tail节点，在http 、server、location、location if配置块中使用,请求访问阶段处理，用于访问控制
- content_by_lua/content_by_lua_file指令,处于content阶段，在lcoation、location if中配置,内容处理器 ，接受请求并处理响应。
- header_filter_by_lua/header_filter_by_lua_file指令,处于out-header-filter阶段，在http 、server、location、location if配置块中使用，用来设置header和cookie
- body_filter_by_lua/header_filter_by_lua_file指令,处于out-body-filter阶段，在http 、server、location、location if配置块中使用，用来过滤响应数据，比如截断或者替换
- log_by_lua/log_by_lua_file指令,处于log阶段，，在http 、server、location、location if配置块中使用，log阶段处理，如记录访问量/统计平均响应时间
- balancer_by_lua_block/balancer_by_lua_file指令，处于content阶段，在upstream配置块中使用，用来设置上游服务器的负载均衡算法
- ssl_certificate_by_lua_block指令，处于content阶段,在server配置块使用，在nginx和上游服务器开始一个SSL握手操作时执行本配置项的lua代码
#####  init_by_lua 指令
- 在每次nginx重新加载配置时候执行,可以用来完成一些耗时的模块的加载，或者初始化一些全局配置。在管理进程创建工作进程时候，此指令中加载的全局变量会进行COW,即复制所有全局变量到worker进程

##### 配置和使用

- 首先需要在nginx.conf的http模块中添加如下代码:

  ```
  http {
    lua_shared_dict shared_data 1m;
    init_by_lua_file init.lua; 
    // server的配置
  }
  ```
- 编写int.lua
  ```
	local redis = require 'resty.redis'
	local cjson = require 'cjson'
	count = 1
	local shared_data = ngx.shared.shared_data
	shared_data:set("count",1)
  ```
- 编写test.lua
  ```
	count = count+1
	ngx.say("global variable : ",count)
	local shared_data = ngx.shared.shared_data
	ngx.say("shared memory :",shared_data:get("count"))
	shared_data:incr("count",1)
	ngx.say("hello world")
  ```
- 配置localtion
  ```
	worker_processes 1;
	events {
  		worker_connections 512;
	}
	http {
  		lua_shared_dict shared_data 1m;
  		init_by_lua_file init.lua; 
  		server {
  			listen 7000;
  			server_name *.*;
  			location /test {
    			content_by_lua_block {
      			ngx.print("hello,first demo for openresty\n")
    		}
  		}
  		location /lua {
    		default_type "text/xml";
    		content_by_lua_file  "test.lua";
  		}
  	  }
	}
  ```

##### 重新加载配置和访问

```
 $ /usr/local/openresty/bin/openresty -s reload
 $ curl 127.0.0.1:7000/lua
 global variable : 2
 shared memory :1
 hello world
$ curl 127.0.0.1:7000/lua
 global variable : 3
 shared memory :2
 hello world
$ curl 127.0.0.1:7000/lua
 global variable : 4
 shared memory :3
 hello world
```

​    