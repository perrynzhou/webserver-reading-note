
## nginx服务器架构探讨

| 作者 | 时间 |QQ技术交流群 |
| ------ | ------ |------ |
| perrynzhou@gmail.com |2020/10/20 |中国开源存储技术交流群(672152841) |


### 通用服务器的请求处理机制

- 多进程,每当服务器端收到一个请求，就由服务器fork一个子进程来处理这个请求；优点是设计简单，每个子进程相互独立互相不干扰，这样保证了稳定性；缺点同样也明显，操作系统fork子进程需要内存复制，在资源和时间上产生一定的开销；
- 多线程，每当服务器端收到一个请求，服务端就创建一个线程来处理请求；优点是，创建线程的代码远远小于创建子进程；缺点是，由于每个线程是共享内存和资源的，所以每个线程之间可能会相互影响，同时会产生共享资源的征用。
- 异步方式,网络通信中同步和异步来描述通信模式的概念。
  - 同步机制是指发送方发送请求后，需要等待接收方发回响应后，才会接着发送下一个请求。在同步机制中，所有的请求在服务端也是同步处理，发送方和接收方对请求处理的步调是一致的。
  - 异步机制是指发送方发送请求后，发送方发送一个请求后，不等待接收方响应这个请求，就继续发送下一个请求，在异步机制中，所有发送方的请求形成一个队列,接收方处理请求后再通知发送方。
  
- 阻塞和非阻塞,它们是用来描述进程的处理调用方式,在网络通信中，主要指socket的阻塞和非阻塞的方式，而socket的实质也是IO操作。socket的阻塞方式调用结果返回之前M当前线程从运行态被挂起,一直等到调用结果返回之后，才进入就绪状态，获取CPU继续执行；socket的非阻塞调用方式和阻塞方式正好相反，在非阻塞方式中,如果调用结果不能立即返回，当前线程不会被挂起,而是立即返回执行下一个调用。
  - 同步阻塞,发送方向接收方发请求后，一直的等待响应；接收方处理请求时进行IO操作如果不能立即得到返回结果，就一直等到返回结果，才响应发送方，期间不能进行其他的工作
  - 同步非阻塞,发送方向接收方发送请求后，一直等待响应，接收方处理请求时进行的IO操作如果不能立即得到结果,就立即返回，去做其他的事情，但是由于没有得到请求处理结果，不能响应发送方，发送方一直等待，一直到IO操作完成后，接收方获得结果响应发送方后，接收方才进入下一个请求的过程。
  - 异步阻塞,发送方向接收方发送请求后，不用等待响应，发送方接着处理其他的工作，接收方处理请求时进行的IO操作如果不能立即得到结果,就一直等待，直到IO操作完成后，接收方获得结果响应发送方后。
  - 异步非阻塞,发送方向接收方发送请求后，不用等待响应，发送方接着处理其他的工作，接收方处理请求时进行的IO操作如果不能立即得到结果,也不等待而是立即返回去做其他的事情，当IO操作完成后，将完成状态和结果通知发送方，接收方才响应发送方。
- nginx采用异步非阻塞方式,nginx中work进程为服务端的处理进程，每个work进程可以处理多个客户端请求,当某个work进程接受到客户端的请求后，调用IO进行处理，如果不能马上得到响应结果，就去处理其他的事情；而客户端也无需等待响应，可以处理其他的事情；当IO调用返回完成时,就会通知工作进程，这时候工作进程挂起当前事务的处理，去响应客户端请求。这里有一个问题，服务端work进程IO调用如果把自己状态通知到wo进程？一般有2中解决方法，第一种是，让、work进程在进行其他工作过程中间隔一段时间去检查下IO的运行状态，如果未完成，就继续正在进行的工作；第二种是IO调用在完成后主动通知work进程，对于前者，虽然工作进程在IO调用过程中没有等待，但是需要不断的检查IO调用状态，这种效率不高。linix提供select/poll/epoll等，nginx中主要使用epoll。

### linux IO模型

![linux-io-mode](../images/linux-io-mode.jpg)
### 整体架构

![nginx-process-strucual](../images/nginx-process-structre.png)

- 简单介绍
  - nginx最基本的进程有master进程和work进程，还可能会有cache相关进程。master进程和work进程之间进行通信来传递工作进程的控制信息。
  - nginx强大的模块功能，和外界服务器通信非常便捷，比如nginx的upstream和后端的Web服务器通信，依靠fastcgi与后端应用服务器通信
  
- 核心进程模型
  - nginx进程模型和大多数后台服务一样，分为管理进程和工作进程。管理进程主要是负责管理和监控工作进程，同时充当真个进程组合用户的交互接口;工作进程则是处理用户的这是请求。nginx启动后主进程充当了管理进程，而由管理进程fork出来的子进程充当了工作进程。
  - nginx多进程模型的入口是在ngx_master_process_cycle函数上，该函数做完信号处理设置后，会调用ngx_start_worker_processes函数用于fork子进程，每个子进程里面执行ngx_worker_process_cycle函数来处理请求，该函数是一个for(;;)循环，不断的处理来自客户端的请求，而主进程则继续执行gx_master_process_cycle.
  - 每个工作进程都有有一个for(;;)循环,以便进程持续等待和处理自己的负责的事务

	```
	//启动主进程的函数，该函数就是为了创建nginx的工作进程
	static void ngx_start_worker_processes(ngx_cycle_t *cycle, ngx_int_t n, ngx_int_t type)
	{
		    for (i = 0; i < n; i++) {
		
		    ngx_spawn_process(cycle, ngx_worker_process_cycle,
		                          (void *) (intptr_t) i, "worker process", type);
		}
	}
	//fork的封装函数，用户fork进程和在子进程中执行proc函数
	ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle, ngx_spawn_proc_pt proc, void *data,char *name, ngx_int_t respawn)
	{
		ngx_pid_t  pid;
		pid = fork();
		switch (pid) {
		case 0:
		    ngx_parent = ngx_pid;
		    ngx_pid = ngx_getpid();
		    proc(cycle, data);
		    break;
		
		default:
		    break;
		}
	}
	//nginx的工作进程
	static void ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data)
	{
		        //工作进程的初始化
		        ngx_worker_process_init(cycle, worker);
		        //nginx的event时间处理函数
		        ngx_process_events_and_timers(cycle)
		        {
		            (void) ngx_process_events(cycle, timer, flags);
		        }
	}
	```
- 监控管理进程(master进程)
  - 监控进程的无限for(;;)循环中的sigsuspend()函数，该函数使得监控管理进程大部分时间是在挂起等待状态，知道监控进程收到信号为止.每当接受到用户发送到master进程的信号，唤醒sigsuspend函数不阻塞，nginx_master_process_cycle根据不同信号来处理不同的行为。
  
	```
	void ngx_master_process_cycle(ngx_cycle_t *cycle)
	{
	//在进程进程里面启动工作进程
   	ngx_start_worker_processes(cycle, ccf->worker_processes, NGX_PROCESS_RESPAWN);
	//启动cache管理进程
    ngx_start_cache_manager_processes(cycle, 0);
	for ( ;; ) {
		  //会一直阻塞到这里，直到收到来自用户的信号
		  sigsuspend(&set);
		  //是否有子进程退出
		  if (ngx_reap) {
			  //对应的处理函数
			  ngx_reap_children(cycle);
		  }
		  //是否接受到nginx退出的信号
		  if (!live && (ngx_terminate || ngx_quit)) {
            ngx_master_process_exit(cycle);
        }
		//是否是nginx退出的信号
		if (ngx_terminate) {
			ngx_signal_worker_processes(cycle, SIGKILL);
		}
		//是否收到nignx优雅退出的信号
		if (ngx_quit) {
            ngx_signal_worker_processes(cycle,ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
		}
		//是否收到配置文件重新加载的信号
		if (ngx_reconfigure) {
			//重新加载配置信号收到以后，重新启动新的子进程作为工作进程
			ngx_start_worker_processes(cycle, ccf->worker_processes,NGX_PROCESS_JUST_RESPAWN);
			//启动cache管理进程
            ngx_start_cache_manager_processes(cycle, 1);
			//关闭原来旧的工作进程
			ngx_signal_worker_processes(cycle,ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
		}
		if (ngx_reopen) {
            ngx_signal_worker_processes(cycle,ngx_signal_value(NGX_REOPEN_SIGNAL));
        }

        if (ngx_change_binary) {
            ngx_new_binary = ngx_exec_new_binary(cycle, ngx_argv);
        }

        if (ngx_noaccept) {
            ngx_signal_worker_processes(cycle,ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
        }
	}
	}
	```
- 工作进程(worker process)
  - 工作进程主要关注和客户端或者后端服务器(作为反向代理)之间的数据可读可写的IO事件交互而不是进程信号，所有的工作进程的阻塞点是在像select()/epoll_wait()这样的IO复用函数。
  
  ```
  static void ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data)
	{
    	//nginx工作进程的初始化
    	ngx_worker_process_init(cycle, worker);
		//启动cache管理进程
    	ngx_setproctitle("worker process");
    	for ( ;; ) {
        	if (ngx_exiting) {
            	if (ngx_event_no_timers_left() == NGX_OK) {
                	ngx_worker_process_exit(cycle);
            	}
        	}
       	//nginx的事件和定时器的处理函数
        ngx_process_events_and_timers(cycle)
		{
			//这里调用ngx_epoll_process_events函数处理IO
			ngx_epoll_process_events()
		}
        if (ngx_terminate) {
            ngx_worker_process_exit(cycle);
        }
        if (ngx_quit) {

            if (!ngx_exiting) {
                ngx_close_listening_sockets(cycle);
                ngx_close_idle_connections(cycle);
            }
        }
        if (ngx_reopen) {
            	ngx_reopen_files(cycle, -1);
        	}
    	}
	}
  ```
  ```
	[root@CentOS1 ~]$ gdb /usr/local/nginx/sbin/nginx
	(gdb) !ps -ef|grep nginx
	root     20899     1  0 02:29 ?        00:00:00 nginx: master process /usr/local/nginx/sbin/nginx
	nobody   20900 20899  0 02:29 ?        00:00:00 nginx: worker process
	root     20939 20857  0 02:31 pts/1    00:00:00 gdb /usr/local/nginx/sbin/nginx
	root     20944 20939  0 02:31 pts/1    00:00:00 bash -c ps -ef|grep nginx
	root     20946 20944  0 02:31 pts/1    00:00:00 grep nginx
	(gdb) attach  20900
	A(gdb) br ngx_process_events_and_timers
	Breakpoint 1 at 0x43f26e: file src/event/ngx_event.c, line 199.
	(gdb) br ngx_worker_process_cycle 
	Breakpoint 2 at 0x44c29c: file src/os/unix/ngx_process_cycle.c, line 730.
	(gdb) c
	Continuing.
	//此时在另外一个窗口执行curl http://127.0.0.1/index
	Breakpoint 1, ngx_process_events_and_timers (cycle=0xde0130) at src/event/ngx_event.c:199
	199         if (ngx_timer_resolution) {
	(gdb) c
	Continuing.

	Breakpoint 1, ngx_process_events_and_timers (cycle=0xde0130) at src/event/ngx_event.c:199
	199         if (ngx_timer_resolution) {
	(gdb) n
	204             timer = ngx_event_find_timer();
	(gdb) macro expand ngx_process_events
	expands to: ngx_event_actions.process_events
	(gdb) p ngx_event_actions.process_events
	$1 = (ngx_int_t (*)(ngx_cycle_t *, ngx_msec_t, ngx_uint_t)) 0x44e497 <ngx_epoll_process_events>
  ```