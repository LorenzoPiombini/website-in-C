#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>	 
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "com.h"
#include "http_header.h"
#include "pages.h"


#define MAX_PENDING 5
#define HTTP_HEADER 8190

int main(void){
	char* port = "443";
#if 0	
	struct sockaddr_in server_addr, client_addr;

	/* zero out the address struct*/
	memset(&server_addr, 0, sizeof(server_addr));
#endif	
	/* set up the header */	
	HttpHeader response_t = {0};
	response_t.http_v = "HTTP/1.1";
	response_t.status = 200;
	response_t.content_t = CONTENT;
	response_t.cache_cntl = CACHE;
#if 0
	int sk_d = -1; 
	
	if(( sk_d = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		 perror("soket failed");
		 return -1;
	}	

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	
	if(bind(sk_d, (const struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) 
	{
		printf("connection failed");
		return -1 ;
	}
	
	if(listen(sk_d, MAX_PENDING)  == - 1)
	{
		perror("socket: \n");
		return -1;
	}
#endif
	if(start_SSL(&ctx,port) == -1) {
		return -1;
	}
	

	printf("listening on port %s...\n", port);
		
	int cliLen = 0;
	int sk_cl_d = -1;
	for(;;)
	{
		BIO *client_bio;
		SSL *ssl_n;
		size_t bread;

		//cliLen = sizeof(client_addr);
		
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

		/*if((sk_cl_d = accept(sk_d, (struct sockaddr*) &client_addr, &cliLen)) == - 1){
			perror("client socket: \n");
			return -1;
		}*/

		printf("client connected.\n");
				
		char request[8000];		
		
		int req = SSL_read_ex(ssl_n,request,sizeof(request),&bread);
		
		/*if(req < 0)
		{
			perror("recv failed: ");
			continue;	
		} */

		if(req == 0)
		{
			printf("read failed\n");
			SSL_free(ssl_n);
			continue;
		}

		request[req]= '\0';
		printf("%s",request);
		char* index_pg = NULL;
		int page_size = index_html(&index_pg);
		char response[1016 + page_size];
		int response_size = 0;

		if((response_size = snprintf(response,1016+page_size,"%s %d %s\r\n"\
				    "Content-type: %s\r\n"\
			    	    "Content-length: %d\r\n"\
			            "Connection: keep-alive\r\n"\
				    "\r\n"\
				    "%s"
				    ,response_t.http_v,response_t.status,"OK",
					 response_t.content_t,page_size,index_pg)) < 0)
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

