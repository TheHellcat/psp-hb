
typedef struct intercomDataHeader
{
	int dataSize;
	int dataSeed;
	int mseedcount;
	int mseeds[16];
	int mchunkSize;
	int usecustomkey;
	unsigned char customkey[16];
} intercomDataHeader;
