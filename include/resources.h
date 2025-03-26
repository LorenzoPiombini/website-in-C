#ifndef PAGES_H
#define PAGES_H

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

int attack_response(char **page);
int index_html(char **page);
int load_file(char *file, char **content);


#endif
