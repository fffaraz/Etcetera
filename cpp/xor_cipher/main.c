#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//#include<unistd.h>

#include "xorcipher.h"

void usage(const char *cmd)
{
	fprintf(stderr, "Usage: %s <in-file> [<out-file>]\n", cmd);

	exit(1);
}

int main(int argc, char *argv[])
{
	char *infname;
	char *outfname;
	//const unsigned char *key;
	const unsigned char key[100] = {0};
	FILE *infp, *outfp;
	const int buffer_size = 1024*1024;
	char *inbuffer;
	char *outbuffer;
	int numr,numw;
	
	if(argc<2) usage(argv[0]);
	infname = argv[1];
	
	if(!(infp=fopen(infname, "rb"))) 
	{
		fprintf(stderr, "ERROR: fopen(%s)\n", argv[1]);
		exit(1);
	}
	
	if(argc>2) outfname=argv[2];
	else 
	{
		if(!(outfname=(char*)malloc(strlen(infname)+5))) 
		{
			fprintf(stderr, "ERROR: malloc failed\n");
			exit(1);
		}
		strcpy(outfname, argv[1]);
		strcat(outfname, ".xor");
	}
	
	if(!(outfp=fopen(outfname, "wb"))) 
	{
		fprintf(stderr, "ERROR: fopen(%s)\n", outfname);
		exit(1);
	}
	
	//key = (const unsigned char*)getpass("Please enter a key: ");
	printf("Please enter a key: ");
	scanf("%s", key);
	
	inbuffer = (char*)malloc(buffer_size);
	if(!inbuffer) 
	{
		fprintf(stderr, "ERROR: buffer malloc failed\n");
		exit(1);
	}
	
	outbuffer = (char*)malloc(buffer_size);
	if(!outbuffer) 
	{
		fprintf(stderr, "ERROR: buffer malloc failed\n");
		exit(1);
	}
	
	while(!feof(infp))
	{
		numr=fread(inbuffer,1,buffer_size,infp);
		if(numr!=buffer_size)
		{
			if(ferror(infp)!=0)
			{
				fprintf(stderr,"read file error.\n");
				exit(1);
			}
			else if(feof(infp)!=0);
		}
		
		xorcipher(key, inbuffer, outbuffer, numr);
		
		numw=fwrite(outbuffer,1,numr,outfp);
		if(numw!=numr)
		{
			fprintf(stderr,"write file error.\n");
			exit(1);
		}
	}

	fclose(outfp);
	fclose(infp);
	
	free(inbuffer);
	free(outbuffer);
	if(argc<3) free(outfname);
	
	return 0;
}
	