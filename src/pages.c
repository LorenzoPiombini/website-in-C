#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pages.h"
#include "http_header.h"

int load_file(char *file, char **content)
{
	FILE *fp = fopen(file,"rb");
	if(!fp) {
		fprintf(stderr,
				"can't open %s", file);
		return -1;
	}

	fseek(fp,0,SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	*content = calloc(size,sizeof(char));
	if(!(*content)) {
		fprintf(stderr,"calloc error.\n");
		fclose(fp);
		return -1;
	}
	
	if(fread(*content,size,1,fp) == 0) {
		fprintf(stderr,"can't read from the file %s:%d,\n",
				__FILE__,__LINE__ -2);
		fclose(fp);
		return -1;
	}
	
	fclose(fp);
	return (int)size;
}



int index_html(char** page)
{
	char *index_pg = "<!DOCTYPE html>\n<html>\
			  <head>\
			  \n<meta charset=\"UTF-8\">\
			  \n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
			  <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH\" crossorigin=\"anonymous\">\
			  \n<style>\
			  \nbody {\n\
			    background-color: linen;\n\
			 }\n\
			 h1 {\n\
                         \tcolor: maroon;\n\
                         \tmargin-left: 40px;\n\
                         }\n\
			 .c-margin {\n\
			 \tmargin-left:40px;\n\
			 \tmargin-right:40px;\n\
			 }\n\
                         </style>\n\
                         </head>\n\
                         <body>\n"\
			 NAV_BAR\
			 "<h1 class=\"text-center\">#include&ltWelcome!!.h&gt</h1>\n\
                         <h3 class=\"text-center\">This is a website entiring made with the C programming language!</h3>\
                         <h3 class=\"text-center\">Well, kind of... Let me explain! Everything you see on this page is loaded via</h3>\
                         <h3 class=\"text-center\">a C program on a droplet, the software is a \"custom\" HTTP server, that receive request from browsers and send back the response! </h3>\
                         <h3 class=\"text-center\">there are no html, CSS and .js files, everything is a C string \"callocated\" and sent to this browser, then the browser will do the job.</h3><br>\
                         <h3 class=\"text-center\"><a href=\"https://github.com/LorenzoPiombini/website-in-C.git\">click here to see the source code </a></h3><br>\
                         <h3 class=\"text-center\">this are three projects that I am working on</h3>\
			 <div class=\"row justify-content-center c-margin\">\n\
			 <div class=\"col-md-4\">\n"\
			 CARD_HEAD\
			 CARD_CONT_a\
			 CARD_TAIL\
			"</div>\n\
			<div class=\"col-md-4\">\n"\
			 CARD_HEAD\
			 CARD_CONT_b\
			 CARD_TAIL\
			"</div>\n\
			<div class=\"col-md-4\">\n"\
			 CARD_HEAD\
			 CARD_CONT_c\
			 CARD_TAIL\
			"</div>\n\
			</div>\n\
			</div>\n\
			  <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz\" crossorigin=\"anonymous\"></script>\
                          </body></html>";

	size_t buffer = strlen(index_pg) + 1;
	*page = calloc(buffer, sizeof(char));
	
	if(!(*page)){
		fprintf(stderr,"calloc fialed %s:%d",
				__FILE__, __LINE__ - 3);
		return -1;
	}	
		
	strncpy(*page,index_pg,buffer);

	/*return the page size */
	return buffer;
}


int attack_response(char **page)
{
	char *attack_page = "<!DOCTYPE html>\n<html>\
			  <head>\
			  <title>EFF YOU!!</title>\
			  \n<meta charset=\"UTF-8\">\
			  \n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
			  <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH\" crossorigin=\"anonymous\">\
			  \n<style>\
			  \nbody {\n\
			    background-color: linen;\n\
			 }\n\
			 h1 {\n\
                         \tcolor: maroon;\n\
                         \tmargin-left: 40px\n;\
                         }\n\
                         </style>\n\
                         </head>\n\
                         <body>\n\
			 <h1>DON'T BE A BAD BOT...</h1>\n\
			 <h2> I can hack you back...and you won't like it! </h2>\n\
                         </body>\n\
			 <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz\" crossorigin=\"anonymous\"></script>\
			 </html>\n";


	size_t page_size = strlen(attack_page) + 1;
	*page = calloc(page_size,sizeof(char));
	if(!(*page)) {
		fprintf(stderr,"calloc failed. %s:%d.\n",
				__FILE__,__LINE__ - 2);
		return -1;
	}
	
	strncpy(*page,attack_page,page_size);

	return page_size;
}
