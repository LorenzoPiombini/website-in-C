#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pages.h"




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
                         \tmargin-left: 40px\n;\
                         }\n\
                         </style>\n
                         </head>\n\
                         </body>\n\
                         <h1>#include&ltWelcome!!.h&gt</h1>\n\
                         <h3>This is a website entiring made with the C programming language!</h3>\
			 <a href=\"https://github.com/LorenzoPiombini/website-in-C.git\">click here to see the repo </a>\
			  <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz\" crossorigin=\"anonymous\"></script>\
                          </body></html>";
	size_t buffer = strlen(index_pg);
	*page = calloc(buffer, sizeof(char));
	
	if(!page)
	{
		printf("calloc fialed %s:%d", __FILE__, __LINE__ - 3);
		return -1;
	}	
		
	strncpy(*page,index_pg,buffer-1);
	(*page)[buffer - 1]= '\0';	
	return buffer - 1;
}
