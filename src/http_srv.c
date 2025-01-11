#include <stdio.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <string.h>
#include <unistd.h>
#include "com.h" /* for start_SSL */
#include "http_header.h"
#include "pages.h"



int main(void){
	char* port = "443";

	/* set up the header */	
	HttpHeader response_t = {0};
	response_t.http_v = "HTTP/1.1";

	if(start_SSL(&ctx,port) == -1) {
		return -1;
	}
	

	printf("listening on port %s...\n", port);
		
	for(;;)
	{
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
		

		request[bread-1]= '\0';
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
					 response_t.content_t,page_size-1,page)) <= 0) {
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
			
		}

		response_t.status = OK;
		response_t.content_t = CONTENT;
		response_t.cache_cntl = CACHE;

		char* index_pg = NULL;
		int page_size = 0;
		if((page_size = index_html(&index_pg)) == -1) {
			/*TODO send a bad request response */	
			SSL_free(ssl_n);
			continue;
		}	
		char response[1016 + page_size];
		if((response_size = snprintf(response,1016+page_size,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "Cache-Control: %s\r\n"\
				    "\r\n"\
				    "%s"
				    ,response_t.http_v,response_t.status,"OK",
					 response_t.content_t,page_size-1,
					 response_t.cache_cntl,index_pg)) <= 0) {
			printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
			SSL_free(ssl_n);
			free(index_pg);
			return -1;
		}
	
		if(SSL_write_ex(ssl_n,response,response_size,&bwritten) == -1)
		{
			perror("recieved failed\n");
			SSL_free(ssl_n);
			free(index_pg);
			return -1;
		}
		
		free(index_pg);
		SSL_free(ssl_n);
			
	}
	
	return 0;
}

