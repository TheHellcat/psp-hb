/*
 *  misc functions to support the app
 *
 *  header
 *
 ***/

int hcMiscMenuShowMenu( int xpos, int ypos );
void hcMiscMenuSetColors( int norm, int sel, int desc );
void hcMiscMenuAddEntry( char* caption, int id , char* desc);
void hcMiscMenuClear( void );
void hcMiscMenuSetName( char* MenuName );
void hcMiscWaitForAnyButton( void );
void hcMiscSeqFindFile( u8* buffer, int bufsize, char* filename, int* start, int* size );
int hcMiscSeqGetFile( u8* buffer, int bufsize, char* filename, u8* destbuffer );
