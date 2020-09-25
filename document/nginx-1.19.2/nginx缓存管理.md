###  Cache管理进程

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/05/24 |


- 如果nnginx开启了缓存功能，比如proxy cache,那么nginx还将创建另外两个cache相关的进程，一个是cache manager process，Cache管理进程;另外一个是cache loader process,缓存加载进程;cache进程不处理客户端请求，也不监控IO事件，而处理的是超时的事件，在ngx_process_events_and_timers函数执行ngx_event_expire_timers函数。相关的配置如下
  
	```
	//nginx.conf配置
	http {
	//配置新增缓存目录
   	proxy_cache_path /tmp levels=1:2 keys_zone=one:10m;
    server {
		//忽略
        location / {
			//开启缓存
            proxy_cache one;
			//缓存有效性配置
            proxy_cache_valid 200 302 10m;
            root   html;
            index  index.html index.htm;
        }
	 }
	}
	```
	```
	//未开启cache的进程视图
	[root@CentOS1 ~]$ ps auxf |grep nginx|grep -v grep
	root     20899  0.0  0.0  20868   608 ?        Ss   02:29   0:00 nginx: master process /usr/local/nginx/sbin/nginx
	nobody   20900  0.0  0.0  21304  1640 ?        S    02:29   0:00  \_ nginx: worker process
	```
- cache管理进程启动视图
    ```
    //修改配置，开启缓存功能，重新加载nginx
	[root@CentOS1 ~]$ /usr/local/nginx/sbin/nginx -s reload
	//查看当前的进程视图
	[root@CentOS1 ~]$ ps auxf |grep nginx|grep -v grep     
	root     20899  0.0  0.0  31244  1640 ?        Ss   02:29   0:00 nginx: master process /usr/local/nginx/sbin/nginx
	nobody   21065  0.0  0.0  31664  1244 ?        S    02:44   0:00  \_ nginx: worker process
	nobody   21066  0.0  0.0  31244  1076 ?        S    02:44   0:00  \_ nginx: cache manager process
	nobody   21067  0.0  0.0  31244  1076 ?        S    02:44   0:00  \_ nginx: cache loader process
    ```
- cache管理进程的职责
  -  cache管理进程核心任务就是清理超时的临时缓存文件，限制缓存文件的总大小，这个操作反复执行，直到整个nginx 退出
 	 ```
	 //定义cache管理进程核心结构的ngx_cache_manager_ctx初始化
	 //ngx_cache_manager_ctx为ngx_cache_manager_process_cycle参数
 	 static ngx_cache_manager_ctx_t  ngx_cache_manager_ctx = {
    	ngx_cache_manager_process_handler, "cache manager process", 0
	    };
	    //核心调用链
	    ngx_cache_manager_process_cycle->ngx_process_events_and_timers->ngx_event_expire_timers->ngx_cache_manager_process_handler（ev->handler(ev));

	    //ngx_cache_manager_process_cycle设置定时器触发的函数
	    static void ngx_cache_manager_process_cycle(ngx_cycle_t *cycle, void *data)
	    {
		    ngx_cache_manager_ctx_t *ctx = data;
		    ngx_event_t   ev;
    	    ev.handler = ctx->handler;
    	    ngx_add_timer(&ev, ctx->delay);
    	    for(;;)
    	    {
    		    ngx_process_events_and_timers(cycle);
		    }
	    }
        ```
  - cache管理进程中的ngx_cache_manager_process_handler说明
	```
	/处理每一个磁盘缓存对象的manage函数。然后重新设置磁盘对象下一次超时的时刻返回
	static void ngx_cache_manager_process_handler(ngx_event_t *ev)
	{
    	for (i = 0; i < ngx_cycle->paths.nelts; i++) {
        	if (path[i]->manager) {
            	//manager函数为ngx_http_file_cache_manager函数
            	//nginx调用ngx_http_file_cache_set_slot解析配置指令指令proxy_cache_path来设置回调函数。
            	//manager做两件事情，第一是删除已经过期的缓存文件，然后检查缓存文件是否超过总大小，如果超过则强制进行删除
            	n = path[i]->manager(path[i]->data);
        	}
    	}
	}

	//----------cache manager和loader对应函数的初始化
	//path[i]->manager(path[i]->data)函数的设定
	static ngx_command_t  ngx_http_proxy_commands[] = {
 	{ ngx_string("proxy_cache_path"),
      	NGX_HTTP_MAIN_CONF|NGX_CONF_2MORE,
		//proxy_cache_path命令的配置函数
      	ngx_http_file_cache_set_slot,
      	NGX_HTTP_MAIN_CONF_OFFSET,
      	offsetof(ngx_http_proxy_main_conf_t, caches),
      	&ngx_http_proxy_module },
	}
	//为缓存管理和加载进程中的manager和loader函数初始化
	char *ngx_http_file_cache_set_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
	{
    	ngx_http_file_cache_t  *cache, **ce;
    	cache->path->manager = ngx_http_file_cache_manager;
    	cache->path->loader = ngx_http_file_cache_loader;
    	cache->path->data = cache;
    	cache->path->conf_file = cf->conf_file->file.name.data;
    	cache->path->line = cf->conf_file->line;
    	cache->loader_files = loader_files;
    	cache->loader_sleep = loader_sleep;
    	cache->loader_threshold = loader_threshold;
    	cache->manager_files = manager_files;
    	cache->manager_sleep = manager_sleep;
    	cache->manager_threshold = manager_threshold;
	}
	static ngx_msec_t ngx_http_file_cache_manager(void *data){
	  	//删除已经过期的缓存文件件
    	next = (ngx_msec_t) ngx_http_file_cache_expire(cache) * 1000;
    	for ( ;; ) {
				//删除缓存文件超过总大小的缓存文件
            	wait = ngx_http_file_cache_forced_expire(cache);
    	}
	}
	```