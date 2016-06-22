
/*
 *   Recovery Flasher
 *   by ---==> HELLCAT <==---
 *
 *   Function Library
 *   Header Include
 *
 */

u32 pspFindProc(const char* szMod, const char* szLib, u32 nid);
int hcReadFile(char* file, void* buf, int size);
int hcWriteFile(char* file, void* buffer, int size);
void hcExtractUpdaterSepcialPRX( u8* dataPSAR, char* pbppath );
int hcMiscMenuShowMenu( int xpos, int ypos );
void hcMiscMenuSetColors( int norm, int sel, int desc, int disabled );
void hcMiscMenuAddEntry( char* caption, int id , char* desc);
void hcMiscMenuClear( void );
void hcMiscMenuSetName( char* MenuName );
void hcMiscWaitForAnyButton( void );
void hcMiscSeqFindFile( u8* buffer, int bufsize, char* filename, int* start, int* size );
int hcMiscSeqGetFile( u8* buffer, int bufsize, char* filename, u8* destbuffer );
int _moduleCheck( void* _moduleStart, void* _moduleEnd );
void ClearScreen( void );
void PrintNote( char* text );
void ClearNote( void );
int hcCopyFolder( char* path, char* destpath, u8* workbuffer, int buffersize, int verbose );
int hcReadEBOOTList( char* path, char* listbuffer );
void CachePBPVersion( char* version );
int VerifyPBPVersion( int fw );
int hcCheckForTM( void );
int hcCheckForOE( void );
