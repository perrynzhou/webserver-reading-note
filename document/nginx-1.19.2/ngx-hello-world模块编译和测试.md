
## ngx-hello-world模块编译和测试

| 作者 | 时间 |QQ技术交流群 |
| ------ | ------ |------ |
| perrynzhou@gmail.com |2020/10/28 |中国开源存储技术交流群(672152841) |

- [ngx_http_hello_world_module源代码](./ngx_http_hello_world_module.c)

- ngx_http_hello_world_module的config
  
```
// 模块的config文件/root/nginx-module/ngx-hello-world/config，内容如下
gx_addon_name=ngx_http_hello_world_module

if test -n "$ngx_module_link"; then
  ngx_module_type=HTTP
  ngx_module_name=ngx_http_hello_world_module
  ngx_module_srcs="$ngx_addon_dir/ngx_http_hello_world_module.c"
  . auto/module
else
	HTTP_MODULES="$HTTP_MODULES ngx_http_hello_world_module"
	NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_hello_world_module.c"
fi
```

- 模块安装
```
//ngx-hello-world模块位置在/root/nginx-module/ngx-hello-world，包括ngx_http_hello_world_module.c和config文件
//需要源代码编译
# cd nginx-1.19.2
# ./configure --prefix=/usr/local/nginx --with-debug --with-cc-opt='-ggdb3 -O0' --add-module=/root/nginx-module/ngx-hello-world 
# make && make install
```

- 启动nginx
```
/usr/local/nginx/sbin/nginx  -c /usr/local/nginx/conf/nginx.conf
```

- 配置模块指令

```
 location / {
            hello_world on;
            root   html;
            index  index.html index.htm;
        }
```

```
# /usr/local/nginx/sbin/nginx  -s  reload
```

- 测试模块功能

```
[root@CentOS81 nginx-1.19.2]# curl 127.0.0.1
/*********** perrynzhou ********/
hello world,this is first nginx module
/*********** end ********/
[root@CentOS81 nginx-1.19.2]# 
```