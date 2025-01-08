#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pages.h"




int index_html(char** page)
{
	char *index_pg = "<!DOCTYPE html>\n<html><head>\n<meta charset=\"UTF-8\">\
			  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<style>"\
		         "body {\n"\
  			 "background-color: linen;\n"\
			 "}\n"\
			 "h1 {\n"\
                         "\tcolor: maroon;\n"\
                         "\tmargin-left: 40px\n;"\
                         "}\n"\
                         "</style>\n"
                         "</head>\n"\
                         "</body>\n"\
                         "<h1>#include&ltWelcome!!.h&gt</h1>\n"\
                         "<h3>This is a website entiring made with the C programming language!</h3>"\
			 "<a href=\"https://github.com/LorenzoPiombini/website-in-C.git\">click here to see the repo </a>"
                         "</body></html>";
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
