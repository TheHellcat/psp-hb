#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 *  Another small function to scramble stuff
 *  (it's not really encryption, IMO)
 *  It's by far not unbreakable *lol* - but should be safe enough
 *  for homebrew stuff and other hobby projects.
 *
 *  Remember: It's only as save as the key for (de)scrambling
 *            is protected / stored away
 *
 *  Parameters:  message = pointer to the memory block to scramble
 *               key     = key used for scrambling
 *               msglen  = length (in bytes) of the memoryblock
 *               keylen  = length (in bytes) of the key
 *
 *  Returns:     The memoryblock is scrambled "in place"!
 *               So the source data gets destroyed and the mem block
 *               "message" points to holds the scrambled data then.
 *
 *               The return value of the function is currently unused.
 */
int hcCryptScramble2( unsigned char* message, unsigned char* key, int msglen, int keylen )
{
  int shift;
  int i, j;
  int c;
  unsigned char b;
  int o;
  int t1, t2, t3;
  int keyc;

  shift = key[0] & 0x3;
  if( shift == 0 ) { shift = 5; }

  keyc = 0;
  for( i=0; i<keylen-1; i++)
  {
    keyc += key[i+1] * key[i];
    keyc = keyc << 1;
  }

  c = 0;
  for( i=0; i<msglen; i++ )
  {
    b = message[i];
    t1 = key[c] & 0x0F;
    t2 = (key[c] & 0xF0) >> 4;
    t3 = (t1 * t2) + t1 + t2;
    b = b ^ t3;
    b = b ^ key[c];
    for( j=0; j<t1; j++ )
    {
      o = (b & 0x80) >> 7;
      b = (b << 1) | o;
    }
    if( i == 0 ) { o = 0; } else { o = message[i-1]; }
    b = b ^ o;  // FIX: put more scrambling here, same values = same result
    if( i < msglen ) { o = (keyc >> ((message[i+1] & 0x03)*8)) & 0xFF; } else { o = 0;}
    // FIX (on above statement): more randomness in selecting byte from keyc
    b = b ^ o;
    message[i] = b;
    c++;
    if( c >= keylen ) { c = 0; }
  }

  return 0;
}


int main( void )
{
  char msg[512];
  char key[512];
  int i;
  int lm, lk;
  
  printf( "\n  ---------- test of scramble2 ----------\n\n" );

  sprintf( msg, "XXXXXX XXXXXX\nThis is a test-text that is suposed to be scrambled - some more padding to make it longer than the key ;-)\nXXXXXXXXXX\n\n" );
  sprintf( key, "XXXXXX xyzxyz\n*lol* this is supposed to be the scrambling key xD\n" );
  lm = strlen(msg);
  lk = strlen(key);

  printf( "message length: %i bytes, key length: %i bytes\n\n", lm, lk );

  printf( "\nmessage before scramble:\n" );
  for( i=0; i<lm; i++)
  {
    printf( "%02x", msg[i] );
  }
  printf( "\n" );

  printf( "\nkey:\n" );
  for( i=0; i<lk; i++ )
  {
    printf( "%02x", key[i] );
  }
  printf( "\n" );

  hcCryptScramble2( msg, key, lm, lk );

  printf( "\nmessage after scramble:\n" );
  for( i=0; i<lm; i++)
  {
    printf( "%02x", msg[i] );
  }
  printf( "\n" );

/*  printf( "\n! ATTACK ! trying to fetch key with fake scrambled message\n           of all 0x00 bytes\n\n" );
  memset( msg, 0, lm );
  hcCryptUnscramble2( msg, key, lm, lk );
  for( i=0; i<lm; i++ )
  {
    printf( "%02x", msg[i] );
  }
  printf( "\n" );*/

  return 0;
}
