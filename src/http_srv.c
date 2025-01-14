#include <stdio.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "com.h" /* for start_SSL */
#include "http_header.h"
#include "pages.h"
#include "bst.h"

#define MAX_EVENTS 10
#define PORT 443
#define BLOCK 0

int main(void){

#if BLOCK
	/*port where the server will listen*/
	char* port = "443";

	if(start_SSL(&ctx,port) == -1) {
		fprintf("can't start SSL context.\n");
		return -1;
	}

#else
	/*
	 * initialize the AVL tree
	 * that will store uncompleted handshacke or read
	 * and write SSL operations
	 * */
	BST_init;	
	
	/*epoll setup*/
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];	
	int nfds = 0;
	int epoll_fd = -1;

	/*set up the listening socket */
	int fd_socket = -1;
	if(!listen_set_up(&fd_socket,AF_INET,SOCK_STREAM | SOCK_NONBLOCK,PORT)) {
		printf("listen_set_up() failed %s:%d.\n",F,L-2);
		return -1;
	}


	if(start_SSL(&ctx,"null") == -1) {
		fprintf("can't start SSL context.\n");
		return -1;
	}

	/*start epoll*/
	epoll_fd = epoll_create1(0);
	if(epoll_fd == -1) {
		fprintf(stderr,"can't create epoll.\n");
		SSL_CTX_free(ctx);
		close(fd_socket);
		return -1;
	}

	ev.events = EPOLLIN;
	ev.data.fd = fd_socket;
	
	if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD, fd_socket, &ev) == -1 ) {
		fprintf(stderr,"failed to add fd to  epoll.\n");
		SSL_CTX_free(ctx);
		close(fd_socket);
		return -1;
	}

#endif /* first BLOCK test
	* */
	
	/* set up the header */	
	HttpHeader response_t = {0};
	response_t.http_v = "HTTP/1.1";

	printf("listening on port %d...\n", PORT);
		
	for(;;)
	{
		/*
		 * if you want a blocking server 
		 * set the BLOCK value to 1 and the follwing 
		 * code will be compiled
		 * */
#if BLOCK
		BIO *client_bio;
		SSL *ssl_n;
		size_t bread;
		
		
		if(BIO_do_accept(acceptor_bio) <= 0) 
			continue;
		
		client_bio = BIO_pop(acceptor_bio);
		fprintf(stdout,"client accepted.\n");
		
		if((ssl_n = SSL_new(ctx)) == NULL) {
			fprintf(stderr,"error crienting SSl handle.\n");
			BIO_free(client_bio);
			continue;
		}

		SSL_set_bio(ssl_n,client_bio,client_bio);
		if(SSL_accept(ssl_n) <=0 ) {
			fprintf(stderr,
					"error perform SSL handshake,\n");
			SSL_free(ssl_n);
			continue;
		}

		printf("client connected.\n");
				
		char request[8000];		
		
		int req = SSL_read_ex(ssl_n,request,sizeof(request),&bread);
		
		if(req == 0 || req == -1)
		{
			printf("read failed\n");
			SSL_free(ssl_n);
			continue;
		}
		if(bread == 8000)
			request[bread-1]= '\0';
		else 
			request[bread] == '\0';
#else 	
		char request[8000];
		memset(request,0,8000);

		int client_sock = -1;
		SSL *ssl_cli = NULL;
		nfds = epoll_wait(epoll_fd, events,MAX_EVENTS,-1);
		if(nfds == -1 ) {
			if(errno == EINTR)
				continue;
		}
	
		for(int y = 0; y < nfds; ++y) {
			if(ev[y].events == EPOLLIN) {
				/* */
				int rsl = accept_connection(&ev[y].data.fd,&client_sock,
							request,8000,ctx,
							&ssl_cli, epoll_fd, MAX_EVENTS);
				switch(rsl) {
				case -1:
				case NO_CON:
				case SSL_HD_F:
				case SSL_SET_E:
					continue;
				case SSL_READ_E:
				case HANDSHAKE:
					/*store the client socket 
					 * and the ssl_handle 
					 * of this connection
					 * */
					insert_bts(t_i,(void*)&client_sock,
							&BST_tree.root,
							(void**)&ssl_cli,t_v);
					break;
				default:
					continue;
				}
			} else if(events[y].events == EPOLLOUT){
				/*find the file descriptor in the tree*/



			}
		}	

		printf("%s",request);

		/*check the request to decide what to serve*/
		char *img_buff = NULL;
		long img_size = 0;
		int response_size = 0;
		size_t bwritten;

		if(strstr(request,img_db) != NULL) {
			if(load_image(&img_buff,img_db,&img_size) == -1 ) {
				/*
				 * image not found 
				 * send a 404 response 
				 * */
				response_t.status = NOT_FOUND;
				response_t.content_t = CONTENT_img;
				response_t.cache_cntl = CACHE;
				char response[1016];
				if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"
				    ,response_t.http_v,response_t.status,"Not Found",
					 response_t.content_t,0)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
				}

				if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1)
				{
					perror("recieved failed\n");
					SSL_free(ssl_n);
					free(img_buff);
					continue;
				}
				SSL_free(ssl_n);
				free(img_buff);
				continue;

			}	

			/*send the image along with the response*/
			/*set up response*/
			response_t.status = OK;
			response_t.content_t = CONTENT_img;
			response_t.cache_cntl = CACHE;
			char response[1016];
			if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %ld\r\n"\
			            "Connection: keep-alive\r\n"\
				    "Cache-Control: %s\r\n"\
				    "\r\n"
				    ,response_t.http_v,response_t.status,"OK",
					 response_t.content_t,
					 img_size,response_t.cache_cntl)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
			}
			
			if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(img_buff);
				continue;
			}

			if(SSL_write_ex(ssl_n,img_buff,img_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(img_buff);
				continue;
			}

			SSL_free(ssl_n);
			free(img_buff);
			continue;
		} else if (strstr(request,img_mem) != NULL) {
			if(load_image(&img_buff,img_mem,&img_size) == -1 ) {
				/*image not found */
				response_t.status = NOT_FOUND;
				response_t.content_t = CONTENT_img;
				response_t.cache_cntl = CACHE;
				char response[1016];
				if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"
				    ,response_t.http_v,response_t.status,"Not Found",
					 response_t.content_t,0)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
				}

				if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1)
				{
					perror("recieved failed\n");
					SSL_free(ssl_n);
					free(img_buff);
					continue;
				}
				SSL_free(ssl_n);
				free(img_buff);
				continue;
			}	

			/*send the image along with the response*/
			response_t.status = OK;
			response_t.content_t = CONTENT_img;
			response_t.cache_cntl = CACHE;
			char response[1016];
			if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %ld\r\n"\
			            "Connection: keep-alive\r\n"\
				    "Cache-Control: %s\r\n"\
				    "\r\n"
				    ,response_t.http_v,response_t.status,"OK",
				    response_t.content_t,img_size,
				    response_t.cache_cntl)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
			}
			
			if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1)
			{
				perror("write failed failed\n");
				SSL_free(ssl_n);
				free(img_buff);
				continue;
			}
			if(SSL_write_ex(ssl_n,img_buff,img_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(img_buff);
				continue;
			}
			SSL_free(ssl_n);
			free(img_buff);
			continue;
		} else if (strstr(request,img_usr) != NULL) {
			if(load_image(&img_buff,img_usr,&img_size) == -1 ) {
				/*image not found */
				response_t.status = NOT_FOUND;
				response_t.content_t = CONTENT_img;
				response_t.cache_cntl = CACHE;
				char response[1016];
				if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"
				    ,response_t.http_v,response_t.status,"Not Found",
					 response_t.content_t,0)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
				}

				if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1) {
					perror("recieved failed\n");
					SSL_free(ssl_n);
					free(img_buff);
					continue;
				}
				SSL_free(ssl_n);
				free(img_buff);
				continue;

			}	
			/*send the image along with the response*/
			response_t.status = OK;
			response_t.content_t = CONTENT_img;
			response_t.cache_cntl = CACHE;
			char response[1016];
			if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %ld\r\n"\
			            "Connection: keep-alive\r\n"\
				    "Cache-Control: %s\r\n"\
				    "\r\n"
				    ,response_t.http_v,response_t.status,"OK",
				    response_t.content_t,img_size,
				    response_t.cache_cntl)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
			}
			
			if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(img_buff);
				continue;
			}
			if(SSL_write_ex(ssl_n,img_buff,img_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(img_buff);
				continue;
			}
			SSL_free(ssl_n);
			free(img_buff);
			continue;

		}
	
		if(strstr(request,WPRESS) != NULL || 
		   strstr(request,ROBOT) != NULL) {
			/* serve a funny response to an attacker */
			char *page = NULL;
			int page_size = 0;
			if((page_size = attack_response(&page)) == -1 ){
				/* TODO send a bad request response */	
				SSL_free(ssl_n);
				continue;
			}

			response_t.status = TEAPOT;
			response_t.content_t = CONTENT;
			response_t.cache_cntl = CACHE;
			
			char response[1016 + page_size];
			if((response_size = snprintf(response,1016+page_size,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "Cache-Control: %s\r\n"\
				    "\r\n"\
				    "%s"
				    ,response_t.http_v,response_t.status,"I'm a teapot",
					 response_t.content_t,page_size-1,
					 response_t.cache_cntl,page)) <= 0) {
				fprintf(stderr,"error creating response %s:%d",
						__FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(page);
				continue;
			}
		
			if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1) {
				fprintf(stderr,"recieved failed\n");
				SSL_free(ssl_n);
				free(page);
				continue;
			}
			SSL_free(ssl_n);
			free(page);
			continue;
			
		} else if(strstr(request,GET) != NULL) {

			response_t.status = OK;
			response_t.content_t = CONTENT;
			response_t.cache_cntl = CACHE;
	
			char *index_pg = "index.html";
			char *content = NULL;
			int page_size = 0;
			if((page_size = load_html(index_pg, &content)) == -1) {
				/*TODO send a bad request response */	
				SSL_free(ssl_n);
				continue;
			}	
			char response[1016];
			if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
					    "Content-type: %s\r\n"\
				    	    "Content-length: %d\r\n"\
				            "Connection: keep-alive\r\n"\
					    "Cache-Control: %s\r\n"\
					    "\r\n"\
					    ,response_t.http_v,response_t.status,"OK",
						 response_t.content_t,page_size,
						 response_t.cache_cntl)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(content);
				return -1;
			}
	
			if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(content);
				return -1;
			}
		
			if(SSL_write_ex(ssl_n,content,page_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(content);
				return -1;
			}
			free(content);
			SSL_free(ssl_n);
		}else if(strstr(request,GETabout) != NULL ){
			
			response_t.status = OK;
			response_t.content_t = CONTENT;
			response_t.cache_cntl = CACHE;	
			
			char *about = "about.html";
			char *content = NULL;
			int page_size = 0;
			if((page_size = load_html(about, &content)) == -1) {
				/*TODO send a bad request response */	
				SSL_free(ssl_n);
				continue;
			}	
			char response[1016];
			if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
					    "Content-type: %s\r\n"\
				    	    "Content-length: %d\r\n"\
				            "Connection: close\r\n"\
					    "Cache-Control: %s\r\n"\
					    "\r\n"\
					    ,response_t.http_v,response_t.status,"OK",
						 response_t.content_t,page_size,
						 response_t.cache_cntl)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(content);
				return -1;
			}
	
			if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(content);
				return -1;
			}
		
			if(SSL_write_ex(ssl_n,content,page_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(ssl_n);
				free(content);
				return -1;
			}
			free(content);
			SSL_free(ssl_n);
			continue;
		
		}else {
			response_t.status = NOT_FOUND;
			response_t.content_t = CONTENT;
			response_t.cache_cntl = CACHE;
			
			char response[1016];
			if((response_size = snprintf(response,1016,"%s %d %s\r\n"\
					    "Content-type: %s\r\n"\
				    	    "Content-length: %ld\r\n"\
				            "Connection: close\r\n"\
					    "Cache-Control: %s\r\n"\
					    "\r\n"\
					    "%s"
					    ,response_t.http_v,response_t.status,"Not Found",
						 response_t.content_t,strlen("<h1>NOT FOUND</h1>"),
						 response_t.cache_cntl,"<h1>NOT FOUND</h1>")) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				return -1;
			}
				
			if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(ssl_n);
				return -1;
			}
			SSL_free(ssl_n);
			continue;
		}
	}
	
	return 0;
}

