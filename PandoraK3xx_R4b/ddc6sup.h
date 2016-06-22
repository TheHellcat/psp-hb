
/*
 *   Recovery Flasher
 *   by ---==> HELLCAT <==---
 *
 *   PSAR extraction / FW unpacking and installing
 *   Header Include
 *
 */

void DDC6psarInitPsarDumper( u8* buf1, u8* buf2, int g );
int DDC6psarCopyFwFiles( int checkonly );
void DDC6psarSetPBPPath( char* path );
