## nginx请求处理前需要做哪些事情?

| author | update |
| ------ | ------ |
| perrynzhou@gmail.com | 2020/10/26 |

### nginx如何处理http请求?
- ngx_http_process_request_line:处理客户端发送过来的http请求头中的request-line.这个过程可以分为三步,读取read-line、解析 read-line、存储解析结果并设置相关的值。ngx_http_process_request_line需要解析http基本合法性的请求头和http其他的请求头信息。ngx_http_process_request_line执行逻辑如下

```
static void ngx_http_process_request_line(ngx_event_t *rev)
{
 for ( ;; ) {
   //读取http请求头
 	ngx_http_read_request_header(r);
 	//解析满足http合法性的基本请求头
 	ngx_http_parse_request_line(r, r->header_in);
 	//解析http其他的请求头
 	ngx_http_process_request_headers(rev);
 }
}
```
- 1.读取read-line数据，是通过函数ngx_http_read_request_header将数据读取到r->header_in内，下面是具体读入的内容。
```
(gdb) br ngx_http_read_request_header 
Breakpoint 1 at 0x460a15: file src/http/ngx_http_request.c, line 1513.
(gdb) p *r->header_in
$2 = {pos = 0xf88a40 "User-Agent: Wget/1.14 (linux-gnu)\r\nAccept: */*\r\nHost: 127.0.0.1\r\nConnection: Keep-Alive\r\n\r\n51QANkgDwevcpY8bGAapzyWpW\r\nConnection", 
  last = 0xf88a9b "51QANkgDwevcpY8bGAapzyWpW\r\nConnection", file_pos = 0, file_last = 0, 
  start = 0xf88a30 "GET / HTTP/1.1\r\nUser-Agent: Wget/1.14 (linux-gnu)\r\nAccept: */*\r\nHost: 127.0.0.1\r\nConnection: Keep-Alive\r\n\r\n51QANkgDwevcpY8bGAapzyWpW\r\nConnection", end = 0xf88e30 "", tag = 0x0, file = 0x0, shadow = 0x0, temporary = 1, memory = 0, mmap = 0, recycled = 0, in_file = 0, flush = 0, sync = 0, last_buf = 0, 
  last_in_chain = 0, last_shadow = 0, temp_file = 0, num = 0}

```
- 2.解析Request-line。读取到的request-line数据进行解析工作实现在函数ngx_http_parse_requst_line内，由于requet-line数据有严格的BNF对应，所以解析过程虽然繁琐但是比较好理解

- 3.存储解析结果并设置相关值。在request-line的解析过程中会有一些赋值操作，但是更多是在解析成功后，ngx_http_request_t对象中的r内相关字段都将被赋值。比如uri(/)、method_name(GET)、http_protocol（HTTP/1.0)

- 4.通过ngx_http_read_request_header和ngx_http_parse_requst_line三个步骤，解析http请求的合法性，接下来开始解析其他的请求头，比如general-header、request-header、entity-header。具体的实现函数由ngx_http_process_request_headers进行，分为三步：读取数据(ngx_http_read_request_header)、解析数据(ngx_http_parse_header_line)、存储解析结果
```
(gdb) br ngx_http_process_request_headers
(gdb) p *(ngx_table_elt_t *)(r->headers_in.headers.part.elts+sizeof(ngx_table_elt_t)*0)
$36 = {hash = 2960984997632184587, key = {len = 15, data = 0xf88e68 "X-Forwarded-For"}, value = {len = 27, data = 0xf88e79 "10.208.22.27, 10.101.126.12"}, 
  lowcase_key = 0xf89ec8 "x-forwarded-for"}
(gdb) p *(ngx_table_elt_t *)(r->headers_in.headers.part.elts+sizeof(ngx_table_elt_t)*1)
$37 = {hash = 3208616, key = {len = 4, data = 0xf88e96 "Host"}, value = {len = 18, data = 0xf88e9c "reference.vivo.lan"}, 
  lowcase_key = 0xf89ee0 "hostconnectionx-forwarded-protouser-agentacceptoptraceid/usr/local/nginx/html/check.do"}
(gdb) p *(ngx_table_elt_t *)(r->headers_in.headers.part.elts+sizeof(ngx_table_elt_t)*2)
$38 = {hash = 2715320498552542, key = {len = 10, data = 0xf88ed6 "Connection"}, value = {len = 5, data = 0xf88ee2 "close"}, 
  lowcase_key = 0xf89ee4 "connectionx-forwarded-protouser-agentacceptoptraceid/usr/local/nginx/html/check.do"}
(gdb) p *(ngx_table_elt_t *)(r->headers_in.headers.part.elts+sizeof(ngx_table_elt_t)*3)
$39 = {hash = 4707995373267764650, key = {len = 17, data = 0xf88ee9 "X-Forwarded-Proto"}, value = {len = 4, 
    data = 0xf88efd "ttp\r\nUser-Agent: moniNetAgent_1.0\r\nAccept: */*\r\noptraceid: traceid-cmVmZXJlbmNlLnZpdm8ubGFuLjo4MDgwL2NoZWNrLmRv-1603670063504-7401512130\r\n\r\n\n"}, lowcase_key = 0xf89eee "x-forwarded-protouser-agentacceptoptraceid/usr/local/nginx/html/check.do"}

```

- 5.ngx_http_read_request_headers读取、解析、存储过程中，如果每个请求头有对应的回调函数则会去执行这些回调函数，每个请求的回调函数是在ngx_http_headers_in中设定的。比如解析请求头中的host则会调用ngx_http_headers_in中定义ngx_http_process_host函数
	
```
	static void	ngx_http_process_request_headers(ngx_event_t *rev)
	{
		 for ( ;; ) {
		   ngx_http_read_request_header(r);
		   rc=ngx_http_parse_header_line(r, r->header_in,cscf->underscores_in_headers);
		   //如果ngx_http_parse_header_line解析返回NGX_HTTP_PARSE_HEADER_DONE则代表http的所有请求头已经处理完毕。nginx开始进入到内部处理模块，即开始执行各种hander
		   if (rc == NGX_HTTP_PARSE_HEADER_DONE) {
		   	   //检查http的所有请求头
		       rc = ngx_http_process_request_header(r);
	           ngx_http_process_request(r);
	           break;
		   }
		   hh = ngx_hash_find(&cmcf->headers_in_hash, h->hash,
	                           h->lowcase_key, h->key.len);
	
	       if (hh && hh->handler(r, h, hh->offset) != NGX_OK) {
	            break;
	        }
		 }
	}
```
### nginx处理完http请求头后接着做什么？
- 经过读取http所有请求头，并验证http请求头的合法性后开始进入nginx内部的模块的处理流程，简单的核心逻辑如下
```
static void ngx_http_process_request_line(ngx_event_t *rev)
{
 for ( ;; ) {
   //读取http请求头
 	ngx_http_read_request_header(r);
 	//解析满足http合法性的基本请求头
 	ngx_http_parse_request_line(r, r->header_in);
 	//解析http其他的请求头
 	ngx_http_process_request_headers(rev)
 	{
 		//解析http请求头
 		 rc = ngx_http_parse_header_line(r, r->header_in, cscf->underscores_in_headers);
         if (rc == NGX_HTTP_PARSE_HEADER_DONE) {
            //检查http请求头的合法性
         	rc = ngx_http_process_request_header(r);
            if (rc != NGX_OK) {
                break;
            }
            //开始进入http内部模块处理流程
            ngx_http_process_request(r)
            {
            	//http处理
            	ngx_http_handler(r)
            	{
            		ngx_http_core_run_phases(r);
            	}
            }
            break;
          }
 	}
 }
}
```
### http请求头处理的gdb信息
```
Breakpoint 1, ngx_http_read_request_header (r=0xde9e90) at src/http/ngx_http_request.c:1513
1513        c = r->connection;
(gdb) bt
#0  ngx_http_read_request_header (r=0xde9e90) at src/http/ngx_http_request.c:1513
#1  0x000000000045fbb9 in ngx_http_process_request_line (rev=0xe03bd0) at src/http/ngx_http_request.c:1064
#2  0x000000000045f65b in ngx_http_wait_request_handler (rev=0xe03bd0) at src/http/ngx_http_request.c:500
#3  0x000000000044eac0 in ngx_epoll_process_events (cycle=0xddb230, timer=60000, flags=1) at src/event/modules/ngx_epoll_module.c:901
#4  0x000000000043f5d5 in ngx_process_events_and_timers (cycle=0xddb230) at src/event/ngx_event.c:247
#5  0x000000000044c68f in ngx_worker_process_cycle (cycle=0xddb230, data=0x0) at src/os/unix/ngx_process_cycle.c:740
#6  0x0000000000449652 in ngx_spawn_process (cycle=0xddb230, proc=0x44c5e1 <ngx_worker_process_cycle>, data=0x0, name=0x4d0cef "worker process", respawn=-3)
    at src/os/unix/ngx_process.c:199
#7  0x000000000044b8c0 in ngx_start_worker_processes (cycle=0xddb230, n=1, type=-3) at src/os/unix/ngx_process_cycle.c:349
#8  0x000000000044b0e6 in ngx_master_process_cycle (cycle=0xddb230) at src/os/unix/ngx_process_cycle.c:130
#9  0x000000000040bc5a in main (argc=3, argv=0x7ffc227bc948) at src/core/nginx.c:382
```


### ngx_http_headers_in定义
```
ngx_http_header_t  ngx_http_headers_in[] = {
    { ngx_string("Host"), offsetof(ngx_http_headers_in_t, host),
                 ngx_http_process_host },

    { ngx_string("Connection"), offsetof(ngx_http_headers_in_t, connection),
                 ngx_http_process_connection },

    { ngx_string("If-Modified-Since"),
                 offsetof(ngx_http_headers_in_t, if_modified_since),
                 ngx_http_process_unique_header_line },

    { ngx_string("If-Unmodified-Since"),
                 offsetof(ngx_http_headers_in_t, if_unmodified_since),
                 ngx_http_process_unique_header_line },

    { ngx_string("If-Match"),
                 offsetof(ngx_http_headers_in_t, if_match),
                 ngx_http_process_unique_header_line },

    { ngx_string("If-None-Match"),
                 offsetof(ngx_http_headers_in_t, if_none_match),
                 ngx_http_process_unique_header_line },

    { ngx_string("User-Agent"), offsetof(ngx_http_headers_in_t, user_agent),
                 ngx_http_process_user_agent },

    { ngx_string("Referer"), offsetof(ngx_http_headers_in_t, referer),
                 ngx_http_process_header_line },

    { ngx_string("Content-Length"),
                 offsetof(ngx_http_headers_in_t, content_length),
                 ngx_http_process_unique_header_line },

    { ngx_string("Content-Range"),
                 offsetof(ngx_http_headers_in_t, content_range),
                 ngx_http_process_unique_header_line },

    { ngx_string("Content-Type"),
                 offsetof(ngx_http_headers_in_t, content_type),
                 ngx_http_process_header_line },

    { ngx_string("Range"), offsetof(ngx_http_headers_in_t, range),
                 ngx_http_process_header_line },

    { ngx_string("If-Range"),
                 offsetof(ngx_http_headers_in_t, if_range),
                 ngx_http_process_unique_header_line },

    { ngx_string("Transfer-Encoding"),
                 offsetof(ngx_http_headers_in_t, transfer_encoding),
                 ngx_http_process_unique_header_line },

    { ngx_string("TE"),
                 offsetof(ngx_http_headers_in_t, te),
                 ngx_http_process_header_line },

    { ngx_string("Expect"),
                 offsetof(ngx_http_headers_in_t, expect),
                 ngx_http_process_unique_header_line },

    { ngx_string("Upgrade"),
                 offsetof(ngx_http_headers_in_t, upgrade),
                 ngx_http_process_header_line },

#if (NGX_HTTP_GZIP || NGX_HTTP_HEADERS)
    { ngx_string("Accept-Encoding"),
                 offsetof(ngx_http_headers_in_t, accept_encoding),
                 ngx_http_process_header_line },

    { ngx_string("Via"), offsetof(ngx_http_headers_in_t, via),
                 ngx_http_process_header_line },
#endif

    { ngx_string("Authorization"),
                 offsetof(ngx_http_headers_in_t, authorization),
                 ngx_http_process_unique_header_line },

    { ngx_string("Keep-Alive"), offsetof(ngx_http_headers_in_t, keep_alive),
                 ngx_http_process_header_line },

#if (NGX_HTTP_X_FORWARDED_FOR)
    { ngx_string("X-Forwarded-For"),
                 offsetof(ngx_http_headers_in_t, x_forwarded_for),
                 ngx_http_process_multi_header_lines },
#endif

#if (NGX_HTTP_REALIP)
    { ngx_string("X-Real-IP"),
                 offsetof(ngx_http_headers_in_t, x_real_ip),
                 ngx_http_process_header_line },
#endif

#if (NGX_HTTP_HEADERS)
    { ngx_string("Accept"), offsetof(ngx_http_headers_in_t, accept),
                 ngx_http_process_header_line },

    { ngx_string("Accept-Language"),
                 offsetof(ngx_http_headers_in_t, accept_language),
                 ngx_http_process_header_line },
#endif

#if (NGX_HTTP_DAV)
    { ngx_string("Depth"), offsetof(ngx_http_headers_in_t, depth),
                 ngx_http_process_header_line },

    { ngx_string("Destination"), offsetof(ngx_http_headers_in_t, destination),
                 ngx_http_process_header_line },

    { ngx_string("Overwrite"), offsetof(ngx_http_headers_in_t, overwrite),
                 ngx_http_process_header_line },

    { ngx_string("Date"), offsetof(ngx_http_headers_in_t, date),
                 ngx_http_process_header_line },
#endif

    { ngx_string("Cookie"), offsetof(ngx_http_headers_in_t, cookies),
                 ngx_http_process_multi_header_lines },

    { ngx_null_string, 0, NULL }
};

```