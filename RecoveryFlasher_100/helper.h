
/*
 *   Recovery Flasher
 *   by ---==> HELLCAT <==---
 *
 *   Kernel Helper Module
 *   Header Include
 *
 */
 
void hcRfHelperGetMoboVersions( int* baryon, int* tachyon );
int hcRfHelperLoadStartModule( char *module );
u32 hcRfHelperPspFindProc(const char* szMod, const char* szLib, u32 nid);
int hcRfHelperIoBridgeUnassign( char* dev );
int hcRfHelperRebootToUpdater( void );
int hcRfHelperIoBridgeLFatfmt( int flash );
int hcRfHelperInitMangleSyscall( void );
int hcRfHelperPspEncrypt( u32 *buf, int size );
void hcRfHelperPatchLFatEncryption( void );
int hcRfHelperIoBridgeWriteFile(char* file, void* buffer, int size);
