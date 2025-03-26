#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#define WPRESS "/wp-admin/setup-config.php"
#define ROBOT "/robots.txt"

#define CONTENT "text/html"
#define CONTENT_CSS "text/css"
#define CONTENT_JS "text/javascript"
#define CONTENT_img "image/png"
#define CACHE "private, max-age=3600"


/* statuses */
#define OK 200
#define NOT_FOUND 404
#define TEAPOT 418


typedef struct
{
	char* http_v;
	unsigned short status;
	char* content_t;
	char* cache_cntl; 

}HttpHeader;



#endif
