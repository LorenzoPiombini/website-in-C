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
	response_t.status = 200;
	response_t.content_t = CONTENT;
	response_t.cache_cntl = CACHE;

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

		request[bread]= '\0';
		printf("%s",request);

		/*check the request to decide what to serve*/
		char *img_buff = NULL;
		long img_size = 0;
		int response_size = 0;
		if(strstr(request,img_db) != NULL) {
			if(load_image(&img_buff,img_db,&img_size) == -1 ) {
				/*image not found */
			}	

			/*send the image along with the response*/
			/*set up response*/
			response_t.status = 200;
			response_t.content_t = CONTENT_img;
			response_t.cache_cntl = CACHE;
			char response[1016 + img_size];
			if((response_size = snprintf(response,1016+img_size,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"\
				    "%s"
				    ,response_t.http_v,response_t.status,"OK",
					 response_t.content_t,img_size,img_buff)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
			}
			
			SSL_free(ssl_n);
			free(img_buff);
			continue;
		} else if (strstr(request,img_mem) != NULL) {
			if(load_image(&img_buff,img_mem,&img_size) == -1 ) {
				/*image not found */
			}	

			/*send the image along with the response*/
			response_t.status = 200;
			response_t.content_t = CONTENT_img;
			response_t.cache_cntl = CACHE;
			char response[1016 + img_size];
			if((response_size = snprintf(response,1016+img_size,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"\
				    "%s"
				    ,response_t.http_v,response_t.status,"OK",
					 response_t.content_t,img_size,img_buff)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
			}
			
			SSL_free(ssl_n);
			free(img_buff);
			continue;
		} else if (strstr(request,img_usr) != NULL) {
			if(load_image(&img_buff,img_usr,&img_size) == -1 ) {
				/*image not found */
			}	
			/*send the image along with the response*/
			response_t.status = 200;
			response_t.content_t = CONTENT_img;
			response_t.cache_cntl = CACHE;
			char response[1016 + img_size];
			if((response_size = snprintf(response,1016+img_size,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"\
				    "%s"
				    ,response_t.http_v,response_t.status,"OK",
					 response_t.content_t,img_size,img_buff)) <= 0) {
				printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
				SSL_free(ssl_n);
				free(img_buff);
				return -1;
			}
			
			SSL_free(ssl_n);
			free(img_buff);
			continue;

		}
		char* index_pg = NULL;

		int page_size = index_html(&index_pg);
		char response[1016 + page_size];


		if((response_size = snprintf(response,1016+page_size,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"\
				    "%s"
				    ,response_t.http_v,response_t.status,"OK",
					 response_t.content_t,page_size,index_pg)) <= 0)
		{
			printf("error creating response %s:%d", __FILE__, __LINE__ - 7);
			SSL_free(ssl_n);
			free(index_pg);
			return -1;
		}
	
		size_t bwritten;
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

