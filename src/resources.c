#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "resources.h"
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
