#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"



static int define_method(char *method);
static int define_accept(char *accept);
static int find_char(char *str, const char c);


int parse_request(char *request, struct request_s *req)
{
	size_t size = strlen(request);
	FILE *req_stream = fmemopen(request,size,"r");
	if(!req_stream) {
		fprintf(stderr,"can't 'stream' the reqquest.\n");
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
			if(strlen(token) == 1){
				strncpy((*req).resource, "index.html",strlen("index.html")+ 1);
			}else{
				int index = find_char(token,'?');
				if(index > -1)
					token[index] = '\0';

				size_t l = strlen(token) + 2;
				
				char r[l];
				memset(r,0,l);
				strncpy(r,".",2);
				strncat(r,token,l);
				strncpy((*req).resource,r,l);

			}
		}
	}
	
	if(!(*req).http_v) {
		fprintf(stderr,"http version not supported");
		fclose(req_stream);
		return -1;
	}

	if(strstr((*req).resource,"../") != NULL ||
			strstr((*req).resource,"reload") != NULL) {
		fclose(req_stream);
		return -1;		
	}

	char line[200];
	memset(line,0,200);
	while(fgets(line,200,req_stream)){ 
		if(strstr(line,"Accept: ") != NULL){
			(*req).accept = define_accept(line);
			if((*req).accept == -1) {
				fclose(req_stream);
				return -1;
			}
			memset(line,0,200);
			continue;
		}

		if(strstr(line,"Connection:") != NULL){
			if(strstr(line,"keep_alive") != NULL)
				(*req).keep_alive = 1;

			memset(line,0,200);
			continue;
		}

		memset(line,0,200);
	}

	if((*req).resource[0] == '\0' && strncmp((*req).http_v,"1.1",strlen((*req).http_v)) == 0) {
		fclose(req_stream);
		return -1;
	}

	fclose(req_stream);
	return 0;
}

static int define_accept(char *accept)
{
	if(strstr(accept,"text/html,") != NULL) 
		return HTML;

	if(strstr(accept,"image") != NULL)
		return IMG;
	
	if(strstr(accept,"text/css") != NULL)
		return CSS;

	if(strstr(accept,"*/*") != NULL)
		return JS;

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

static int find_char(char *str, const char c){
	for(int i = 0; str[i] != '\0'; i++, str++ ){
		if(str[i] == c)
			return i;		
	}
	
	return -1;
}
