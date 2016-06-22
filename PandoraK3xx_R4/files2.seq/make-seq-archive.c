/*
 *  very simple tool to copy multiple files into one "archive file"
 *  much like tar
 *
 **/

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "fastlz.c"

char filebuffer[5000000];
char lzbuffer[5000000];
char fileheader[128];


int DoCompress = 1;
int DoScramble = 0;


int main( void )
{
  DIR* d;
  struct dirent* e;
  FILE* outfile;
  FILE* infile;
  int r;
  char s[128];
  
  outfile = fopen( "./files.seq", "a" );
  
  d = opendir( "./files/" );
  if (d != NULL)
  {
    while (e = readdir (d))
	  {
	  	printf("adding %s.... ", e->d_name );
	  	memset(fileheader, 0, 128);
	  	memset(filebuffer, 0, 5000000);
	  	sprintf( s, "./files/%s", e->d_name );
	    infile = fopen( s, "r" );
	    if( infile )
	    {
	      r = fread( filebuffer, 1, 5000000, infile );
	      fclose( infile );
	      if( r > 0 )
	      {
	      	if( DoCompress == 1)
	      	{
	      		printf("(LZ in %i", r);
	      	  r = fastlz_compress(filebuffer, r, lzbuffer);
	      	  printf(" and out %i) ", r);
	      	  memcpy( filebuffer, lzbuffer, r );
	      	}
	        sprintf( fileheader+32, "%s", e->d_name );
	        fileheader[0] = ( r & 0xFF000000 ) / 0x01000000;
	        fileheader[1] = ( r & 0x00FF0000 ) / 0x00010000;
	        fileheader[2] = ( r & 0x0000FF00 ) / 0x00000100;
	        fileheader[3] = ( r & 0x000000FF ) / 0x00000001;
	        fileheader[4] = ( DoCompress & 0x000000FF );
	        fileheader[5] = ( DoScramble & 0x000000FF );
	        fwrite( fileheader, 1, 128, outfile );
	        fwrite( filebuffer, 1, r, outfile );
	        printf("done\n");
	      } else {
	        printf("failed\n");
	      }
	    }
	  }  
    closedir (d);
  }
  
  fclose( outfile );
  return 0;
}
