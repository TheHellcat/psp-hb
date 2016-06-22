
void setkirkworkbuffer( void* buffer );
int encrypt(u8 *buf, int size, int seed);
int decrypt(u8 *buf, int size, int seed);
int encryptky(u8 *buf, int size, int seed, u8* key);
int decryptky(u8 *buf, int size, int seed, u8* key);
void enablekirk( void );
void disablekirk( void );
