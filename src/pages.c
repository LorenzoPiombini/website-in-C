#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pages.h"

#define NAV_BAR "<nav class=\"navbar navbar-expand-lg bg-body-tertiary\">\n\
		<div class=\"container-fluid\">\n\
		<a class=\"navbar-brand\">Lorenzo Piombini</a>\n\
		<button class=\"navbar-toggler\" type=\"button\" data-bs-toggle=\"collapse\" data-bs-target=\"\\#navbarNavAltMarkup\" aria-controls=\"navbarNavAltMarkup\" aria-expanded=\"false\" aria-label=\"Toggle navigation\">\n\
		<span class=\"navbar-toggler-icon\"></span>\n\
		</button>\n\
		<div class=\"collapse navbar-collapse\" id=\"navbarNavAltMarkup\">\n\
		<div class=\"navbar-nav\">\n\
		<a class=\"nav-link active\" aria-current=\"page\" href=\"/\">Home</a>\n\
		<a class=\"nav-link\" href=\"/about\">About</a>\n\
		<a class=\"nav-link\" href=\"/contact\">Contact</a>\n\
		</div>\n\
		 </div>\n\
		</div>\n\
		</nav>\n"

#define CARD_CONT_a "<img src=\"./pexels-cookiecutter-1148820.jpg\" class=\"card-img-top\" alt=\"...\">\n\
	     <div class=\"card-body\">\n\
	     <h5 class=\"card-title\">Isam database</h5>\n\
	     <p class=\"card-text\">This is a filesystem database,\
			inspired from the custom software\
			of the company that I am currently working at.</p>\n\
		    <a href=\"https://github.com/LorenzoPiombini/isam.db-C-language.git\" class=\"btn btn-primary\">Look at the Code</a>\n"

#define CARD_CONT_b "<img src=\"user-3331257_1280.png\" class=\"card-img-top\" alt=\"...\">\n\
			<div class=\"card-body\">\n\
			<h5 class=\"card-title\">Libuser</h5>\n\
			<p class=\"card-text\">A small library to add user programmatically,\
			in your C programs, without exposing the shell or using fork().</p>\n\
			<a href=\"https://github.com/LorenzoPiombini/libuser.git\" class=\"btn btn-primary\">Look at the Code</a>\n"

#define CARD_CONT_c "<img src=\"memory-8141642_1280.jpg\" class=\"card-img-top\" alt=\"...\">\n\
			<div class=\"card-body\">\n\
			<h5 class=\"card-title\">Mem safe</h5>\n\
			<p class=\"card-text\">A library to make C a safer language,\
			I'd like for C to have some cool feature like Rust, and I am working on a library\
			that could henance memory safety in C.</p>\n\
		    <a href=\"\\#\" class=\"btn btn-primary\">Look at the Code</a>\n"

#define CARD_HEAD "<div class=\"card\" style=\"width: 18rem;\">\n"

#define CARD_TAIL "</div>\n</div>\n"


int load_image(char **img_buffer, char *image_name,long *size)
{
	FILE *img = fopen(image_name,"rb");
	if(!img) {
		fprintf(stderr,"image not found");
		return -1;
	}
	
	/*get the size of the image*/
	fseek(img,0,SEEK_END);
	*size = ftell(img);
	rewind(img);

	*img_buffer = calloc(*size,sizeof(char));
	if(!(*img_buffer)) {
		fprintf(stderr,"memory allocation issue\n");
		fclose(img);
		return -1;
	}

	if(fread(*img_buffer,*size,1,img) == 0) {
		fprintf(stderr,"reading image failed.\n");
		free(*img_buffer);
		fclose(img);
		return -1;
	}

	fclose(img);
	return 0;
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
                         \tmargin-left: 40px\n;\
                         }\n\
                         </style>\n\
                         </head>\n\
                         </body>\n"\
			 NAV_BAR\
                         "<h1 class=\"text-center\">#include&ltWelcome!!.h&gt</h1>\n\
                         <h3 class=\"text-center\">This is a website entiring made with the C programming language!</h3>\
                         <h3 class=\"text-center\">Well, kind of! Let me explain ...</h3><br>\
                         <h3 class=\"text-center\">everything you see on this page is loaded via</h3><br>\
                         <h3 class=\"text-center\"> a C program on a droplet, the software is a \"custom\" HTTP server</h3><br>\
                         <h3 class=\"text-center\">that receive request from browsers and send back the response! </h3><br>\
                         <h3 class=\"text-center\">there is no html files, everything is C strings \"callocated\" and sent to this browser.</h3><br>\
                         <h3 class=\"text-center\"><<a href=\"https://github.com/LorenzoPiombini/website-in-C.git\">click here to see the repo </a></h3><br>\
			 <div class=\"row justify-content-center\">\n\
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
