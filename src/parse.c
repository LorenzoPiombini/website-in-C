#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"



static int define_method(char *method);
static int define_accept(char *accept);

int parse_request(char *request, struct request_s *req)
{
	size_t size = strlen(request);
	FILE *req_stream = fmemopen(request,size,"r");
	if(!req_stream) {
		fprintf(stderr,"can't \"stream\" the reqquest.\n");
		return -1;
	}

	size_t buff_size = strcspn(request,"\n")+2;
	char request_line[buff_size];
	memset(request_line,0,buff_size);

	
	while(fgets(request_line,buff_size,req_stream)){ 
		break;
	}


	char *token = strtok(request_line," ");
	if(!token) {
		fprintf(stderr,"can't tokenize request.\n");
		fclose(req_stream);
		return -1;
	}
	
	(*req).method = define_method(token);
	if((*req).method == -1) {
		fprintf(stderr,"method not supported.\n");
		fclose(req_stream);
		return -1;
	}
	
	while((token = strtok(NULL," "))) {
		if(strstr(token,"1.1") != NULL) {
			(*req).http_v = "1.1";
		} else if(strstr(token,"/") != NULL){
			if(strlen(token) == 1)
				(*req).resource = strdup("index");
			else 
				(*req).resource = strdup(&token[1]);
		}
	}
	
	if(!(*req).http_v) {
		fprintf(stderr,"http version not supported");
		fclose(req_stream);
		return -1;
	}

	if(strstr((*req).resource,"/") != NULL) {
		fclose(req_stream);
		return -1;		
	}

	char accept_line[200];
	while(fgets(accept_line,200,req_stream)){ 
		if(strstr(accept_line,"Accept: ") != NULL)
			break;
	}

	(*req).accept = define_accept(accept_line);
	if((*req).accept == -1) {
		fclose(req_stream);
		return -1;
	}

	if((*req).resource && strncmp((*req).http_v,"1.1",strlen((*req).http_v)) == 0) {
		fclose(req_stream);
		return 0;
	}

	fclose(req_stream);
	return -1;
}

static int define_accept(char *accept)
{
	if(strstr(accept,"text/html,") != NULL) 
		return HTML;
	else if	(strstr(accept,"image") != NULL)
		return IMG;
	
	return -1;	
}

static int define_method(char *method)
{
	if(strncmp(method,"GET",strlen(method)) == 0) 
		return GET;
	else if(strncmp(method,"DELETE",strlen(method) == 0))
		return DELETE;
	else if(strncmp(method,"PUT",strlen(method) == 0))
		return PUT;
	else if(strncmp(method,"HEAD",strlen(method) == 0))
		return HEAD;
	else if(strncmp(method,"POST",strlen(method) == 0))
		return POST;
	else if(strncmp(method,"TRACE",strlen(method) == 0))
		return TRACE;
	else if(strncmp(method,"CONNECT",strlen(method) == 0))
		return CONNECT;
	else if(strncmp(method,"PATCH",strlen(method) == 0))
		return PATCH;
	else if(strncmp(method,"OPTIONS",strlen(method) == 0))
		return OPTIONS;
	
	return -1;
}
