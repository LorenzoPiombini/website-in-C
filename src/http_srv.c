#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* for uintptr_t*/
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h> /*for SOCK_STREAM and SOCK_NONBLOCK */
#include "com.h" /* for start_SSL */
#include "http_header.h"
#include "str_op.h"
#include "pages.h"
#include "bst.h"
#include"hash_tbl.h" /*for HashTable*/




#define MAX_EVENTS 10
#define PORT 443
#define BLOCK 0 /*set this to 1 if you want to have a TLS blocking server*/

int main(void){

#if BLOCK
	/*port where the server will listen*/
	char* port = "443";

	if(start_SSL(&ctx,port) == -1) {
		fprintf("can't start SSL context.\n");
		return -1;
	}

#else

	signal(SIGPIPE,SIG_IGN);
	/*
	 * initialize the HashTable  
	 * to keep trak of the connection that have issues (con_i)
	 * with handshakes or SSL_read_ex()
	 * */
	int ht_buckets = 10;
	Node** data_map = calloc(ht_buckets,sizeof(Node*));
	if(!data_map) {
		fprintf(stderr,"can't create data_map for hash table.");
		return -1;
	}
	HashTable ht = {ht_buckets,data_map};


		
	/*epoll setup*/
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];	
	int nfds = 0;
	int epoll_fd = -1;

	/*set up the listening socket */
	int fd_socket = -1;
	if(!listen_set_up(&fd_socket,AF_INET,SOCK_STREAM | SOCK_NONBLOCK,PORT)) {
		printf("listen_set_up() failed %s:%d.\n",
				__FILE__,__LINE__-2);
		destroy_hasht(&ht);
		return -1;
	}


	if(start_SSL(&ctx,"null") == -1) {
		fprintf(stderr,"can't start SSL context.\n");
		destroy_hasht(&ht);
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
		
	char *img_buff_db = NULL;
	char *img_buff_user = NULL;
	char *img_buff_mem = NULL;

	long img_size_db = 0;
	long img_size_user = 0;
	long img_size_mem = 0;
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

		/*check the request to decide what to serve*/
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

#else 	
		/*this is the code for a non blocking tls srver*/
		char request[8000];
		memset(request,0,8000);

		struct con_i *con_info = calloc(1,sizeof(struct con_i));
		if(!con_info) {
			fprintf(stderr,"calloc failed.\n");
			close(fd_socket);
			/*
			 * TODO: look for all the keys in the hash table
			 *	for all the keys you have to translete the number to the adress
			 *	and :
			 *		-deregister the file descriptors from the epoll
			 *		-close the epoll
			 *		-close the client file descriptor
			 *		-free the memory of the address
			 * 
			 * */
			destroy_hasht(&ht);
			return -1;
		}	
		
		struct con_i *info = NULL; 
		int rls = 0;
		int client_sock = -1;
		SSL *ssl_cli = NULL;
		nfds = epoll_wait(epoll_fd, events,MAX_EVENTS,-1);
		if(nfds == -1 ) {
			if(errno == EINTR)
				continue;
		}
	
		for(int y = 0; y < nfds; ++y) {
			if(events[y].events == EPOLLIN || 
					events[y].events == EPOLLET) {
				/* */
				if(events[y].data.fd == fd_socket) {
				rls = accept_connection(&fd_socket,&client_sock,
							request,8000,ctx,
							&ssl_cli, epoll_fd, MAX_EVENTS);
				switch(rls) {
				case -1:
				case NO_CON:
				case SSL_HD_F:
				case SSL_SET_E:
					continue;
				case SSL_READ_E:
				case HANDSHAKE:
				{	/* 
					 * store the follwing 
					 *	-client socket
					 *	-ssl_handle of this connection
					 *	- type of err (HANDSHAKE or SSL_READ_E)
					 * */
					con_info->client_socket = client_sock;
					con_info->ssl_handle = ssl_cli;
					con_info->err = rls;

					/*
					 * convert the address of con_info
					 * in a off_t number and store it 
					 * in the the hashTable
					 * */
					off_t addr_num = (off_t)(uintptr_t)con_info;
					size_t buf = number_of_digit(client_sock)+1;
					char key[buf];
					if(snprintf(key,buf,"%d",client_sock) < 0) {
						fprintf(stderr,"cannot create key");
						/* TODO handle this case*/	
					}

					if(!set(key,addr_num,&ht)) {
						fprintf(stderr,"cannot create key");
						/* TODO handle this case*/	
					}

					break;
				}
				default:
					/*
					 * the read was succesfull
					 * we have a request
					 * */
					break;
				}/*switch*/
				} else {/*if to check fd_socket*/

				/*find the file descriptor in the hashtable*/
				size_t buf = number_of_digit(events[y].data.fd)+1;
				char key[buf];
				if(snprintf(key,buf,"%d",events[y].data.fd) < 0) {
					fprintf(stderr,"cannot create key");
					/* TODO handle this case*/	
				}

				off_t address = 0;
				if((address = get(key,&ht)) == -1) {
					fprintf(stderr,"cannot find file descriptor");
					/* TODO handle this case*/	
				}
				
				info = (struct con_i*)(uintptr_t)address; 
				/*
				 * if we found the file descriptor we try again the
				 * operation that failed earlier,
				 * if it failes again or if the operation
				 * is succesfull we delete all the references 
				 * to this connections and we start the main loop
				 * again,
				 *
				 * */
				if(info->err == HANDSHAKE) {
					/* perform handshake again*/

					if(retry_SSL_handshake(&info->ssl_handle) <= 0) 
						goto clean_up_epollin;
					/*if handshake succeed perform SSL_read_ex()*/
					if((rls = retry_SSL_read(&(info->ssl_handle),
							request,8000)) == -1) 
						goto clean_up_epollin;

					if(rls == SSL_READ_E)
						continue;
				}else if (info-> err == SSL_READ_E) {
					/* perform SSL_read_ex() again*/
					if((rls = retry_SSL_read(&(info->ssl_handle),
							request,8000)) == -1) 
						goto clean_up_epollin;

				}
				/*clean up and restart the loop*/
				clean_up_epollin:
				if(epoll_ctl(epoll_fd,
						EPOLL_CTL_DEL, 
						info->client_socket, NULL) == -1 ) {
					fprintf(stderr,
						"failed to deregister fd from  epoll.\n");
						SSL_free(info->ssl_handle);
						close(info->client_socket);
						free(info);
						return -1;
				}
				Node* node = delete(key,&ht);
				free(node);
				continue;
				}/*end of the if to check which fd cause the EPOLLIN*/
			} else if(events[y].events == EPOLLOUT){
				/*find the file descriptor in the hashtable*/
				size_t buf = number_of_digit(events[y].data.fd)+1;
				char key[buf];
				if(snprintf(key,buf,"%d",events[y].data.fd) < 0) {
					fprintf(stderr,"cannot create key");
					/* TODO handle this case*/	
				}

				off_t address = 0;
				if((address = get(key,&ht)) == -1) {
					fprintf(stderr,"cannot find file descriptor");
					/* TODO handle this case*/	
				}
				
				info = (struct con_i*)(uintptr_t)address; 
				/*
				 * if we found the file descriptor we try again the
				 * operation that failed earlier,
				 * if it failes again or if the operation
				 * is succesfull we delete all the references 
				 * to this connections and we start the main loop
				 * again,
				 *
				 * */
				if(info->err == HANDSHAKE) {
					/* perform handshake again*/

					if(retry_SSL_handshake(&info->ssl_handle) <= 0) 
						goto clean_up;
					/*if handshake succeed perform SSL_read_ex()*/
					if((rls = retry_SSL_read(&(info->ssl_handle),
							request,8000)) == -1) 
						goto clean_up;

					if(rls == SSL_READ_E)
						continue;
				}else if (info-> err == SSL_READ_E) {
					/* perform SSL_read_ex() again*/
					if((rls = retry_SSL_read(&(info->ssl_handle),
							request,8000)) == -1) 
						goto clean_up;

				}
				/*clean up and restart the loop*/
				clean_up:
				if(epoll_ctl(epoll_fd,
						EPOLL_CTL_DEL, 
						info->client_socket, NULL) == -1 ) {
					fprintf(stderr,
						"failed to deregister fd from  epoll.\n");
						SSL_free(info->ssl_handle);
						close(info->client_socket);
						free(info);
						return -1;
				}
				Node* node = delete(key,&ht);
				free(node);
				continue;
			}
		}	
		
		/*
		 * if we failed to get a request 
		 * for what ever reason we restart 
		 * the main loop
		 * */
		if(rls <= SSL_READ_E)
			continue;

		printf("%s",request);

		/*check the request to decide what to serve*/
		int response_size = 0;
		size_t bwritten;

		if(strstr(request,img_db) != NULL) {
			if(!img_buff_db){
				if(load_image(&img_buff_db,img_db,&img_size_db) == -1 ) {
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
					SSL_free(info->ssl_handle);
					close(info->client_socket);
					free(info);
					free(img_buff);
					return -1;
				}

				if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1)
				{
					perror("recieved failed\n");
					SSL_free(info->ssl_handle);
					close(info->client_socket);
					free(info);
					free(img_buff);
					continue;
				}
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				return -1;
			}
			
			if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				continue;
			}

			if(SSL_write_ex(info->ssl_handle,img_buff,img_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				continue;
			}

			SSL_free(info->ssl_handle);
			close(info->client_socket);
			free(info);
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
				SSL_free(ssl_cli);
				free(img_buff);
				return -1;
				}

				if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1)
				{
					perror("recieved failed\n");
					SSL_free(info->ssl_handle);
					close(info->client_socket);
					free(info);
					free(img_buff);
					continue;
				}
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				return -1;
			}
			
			if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1)
			{
				perror("write failed failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				continue;
			}
			if(SSL_write_ex(info->ssl_handle,img_buff,img_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				continue;
			}
			SSL_free(info->ssl_handle);
			close(info->client_socket);
			free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				return -1;
				}

				if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1) {
					perror("recieved failed\n");
					SSL_free(info->ssl_handle);
					close(info->client_socket);
					free(info);
					free(img_buff);
					continue;
				}
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				return -1;
			}
			
			if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				continue;
			}
			if(SSL_write_ex(info->ssl_handle,img_buff,img_size,&bwritten) == -1)
			{
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(img_buff);
				continue;
			}
			SSL_free(info->ssl_handle);
			close(info->client_socket);
			free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(page);
				continue;
			}
		
			if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1) {
				fprintf(stderr,"recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(page);
				continue;
			}
			SSL_free(info->ssl_handle);
			close(info->client_socket);
			free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(content);
				return -1;
			}
	
			if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(content);
				return -1;
			}
		
			if(SSL_write_ex(info->ssl_handle,content,page_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(content);
				return -1;
			}
			free(content);
			SSL_free(info->ssl_handle);
			close(info->client_socket);
			free(info);
		}else if(strstr(request,GETabout) != NULL ){
			
			response_t.status = OK;
			response_t.content_t = CONTENT;
			response_t.cache_cntl = CACHE;	
			
			char *about = "about.html";
			char *content = NULL;
			int page_size = 0;
			if((page_size = load_html(about, &content)) == -1) {
				/*TODO send a bad request response */	
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(content);
				return -1;
			}
	
			if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				free(content);
				return -1;
			}
		
			if(SSL_write_ex(info->ssl_handle,content,page_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(ssl_cli);
				free(content);
				return -1;
			}
			free(content);
			SSL_free(info->ssl_handle);
			close(info->client_socket);
			free(info);
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
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				return -1;
			}
				
			if(SSL_write_ex(info->ssl_handle,response,response_size,&bwritten) == -1) {
				perror("recieved failed\n");
				SSL_free(info->ssl_handle);
				close(info->client_socket);
				free(info);
				return -1;
			}
			SSL_free(info->ssl_handle);
			close(info->client_socket);
			free(info);
			continue;
		}
	}
	
#endif /*second test for BLOCK*/
	return 0;
}

