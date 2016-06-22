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
int DoScramble = 1;


int main( void )
{
  DIR* d;
  struct dirent* e;
  FILE* outfile;
  FILE* infile;
  int r;
  char s[128];
  int i, j;
  char sc1;
  char sc2;
  int cs;
  
  printf( "\n\n  ***\n  ***   mkpk - Make Firmware Package for Recovery Flasher\n  ***\n\n" );
  
  outfile = fopen( "./fw.rfp", "a" );
  
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
	      	if( DoCompress == 1 )
	      	{
	      		printf("(LZ in %i", r);
	      	  r = fastlz_compress(filebuffer, r, lzbuffer);
	      	  printf(" and out %i) ", r);
	      	  memcpy( filebuffer, lzbuffer, r );
	      	}
	      	if( DoScramble == 1 )
	      	{
	      		j=0x11;
	      	  for( i=1; i<=r; i++ )
	      	  {
	      	    sc1 = filebuffer[i-1] & 0x80;
	      	    if( sc1 != 0 ) { sc1 = 1; }
	      	    sc2 = (((filebuffer[i-1] << 1) & 0xFE ) + sc1) ^ 0xAA;
	      	    sc2 ^= j;
	      	    if( j == 0xFF ) { j = 0x11; } else { j += 0x11; }
	      	    filebuffer[i-1] = sc2;
	      	  }
	      	  printf("scrambled ");
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
  
  printf( "\n\n Generating checksum...." );
  
  infile = fopen( "./fw.rfp", "r" );
  
  r = 1;
  cs = 0;
  while( r > 0 )
  {
  	i = 0;
    r = fread( &i, 1, sizeof(i), infile );
    if( r > 0 )
    {
      cs ^= i;
    }
  }
  
  fclose( infile );
  
  printf( " 0x%08x\n\n", cs );
  
  outfile = fopen( "./fw.rfp", "a" );
  fwrite( &cs, 1, sizeof(cs), outfile );
  fclose( outfile );
  
  return 0;
}
