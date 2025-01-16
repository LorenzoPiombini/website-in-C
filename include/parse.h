#ifndef _PARSE_H_
#define _PARSE_H_


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
	IMG

}accpets;

/*http request struct */
struct request_s 
{
	int method;
	char *http_v;
	char *resource;
	int accept;
};

int parse_request(char *request, struct request_s *req);



#endif
