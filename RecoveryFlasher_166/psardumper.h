
/*
 *   Recovery Flasher
 *   by ---==> HELLCAT <==---
 *
 *   PSAR extraction / FW unpacking and installing
 *   Header Include
 *
 */

void psarInitPsarDumper( u8* buf1, u8* buf2, int g );
int psarCopyFwFiles( int checkonly );
void psarSetPBPPath( char* path );
int GenerateSigCheck(u8 *buf);
void psarSetFwFlag( int flag );
