#ifndef _PARSE_H_
#define _PARSE_H_


#define MAX_LENGTH 500 
/*methods enum*/
typedef enum
{
	GET,
	DELETE,
	HEAD,
	OPTIONS,
	CONNECT,
	PATCH,
	POST,
	PUT,
	TRACE
}methods;


typedef enum
{
	HTML,
	IMG,
	CSS,
	JS

}accpets;

/*http request struct */
struct request_s 
{
	int method;
	char *http_v;
	char resource[MAX_LENGTH];
	int accept;
	int keep_alive;
};

int parse_request(char *request, struct request_s *req);



#endif
