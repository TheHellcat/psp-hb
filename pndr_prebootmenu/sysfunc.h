/*
 *
 *
 */

void hcSysfuncFormatF0(void);
void hcSysfuncFormatF1(void);
void hcSysfuncFormatF2(void);
void hcSysfuncMkF1Dirs(void);
void *hcAlignedMalloc(size_t size, size_t align_size);
void hcAlignedFree(void *ptr);
int hcWriteFile(char *file, void *buf, int size);
int hcReadFile(char *file, void *buf, int size);
char* hcFileSelector(char* path, char* filter);
