#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#define WPRESS "/wp-admin/setup-config.php"
#define ROBOT "/robots.txt"
#define GET "/ "

#define CONTENT "text/html;"
#define CONTENT_img "image/png;"
#define CACHE "private, max-age=0"


/* statuses */
#define OK 200
#define NOT_FOUND 404
#define TEAPOT 418

#define img_db "pexels-cookiecutter-1148820.jpg"
#define img_usr "user-3331257_1280.png"
#define img_mem "memory-8141642_1280.jpg"


typedef struct
{
	char* http_v;
	unsigned short status;
	char* content_t;
	char* cache_cntl; 

}HttpHeader;



#endif
