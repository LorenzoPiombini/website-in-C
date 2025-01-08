#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#define CONTENT "text/html;"
#define CACHE "private, max-age=0"
typedef struct
{
	char* http_v;
	unsigned short status;
	char* content_t;
	char* cache_cntl; 

}HttpHeader;



#endif
