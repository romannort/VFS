#define INODE_DIRECT_OFFSETS 13

struct inode 
{
	bool used;
	unsigned long NUMBER;
	unsigned long DATA_NUMBERS[INODE_DIRECT_OFFSETS];
	unsigned long indirect_inode;
};

typedef struct inode Inode;

struct superblock
{
	unsigned long FREE_SIZE;
	unsigned long MAX_SIZE;
	unsigned long INODE_TABLE_START;
	unsigned long INODE_TABLE_SIZE;
	unsigned long DATA_TABLE_START;
	unsigned int CLUSTER_SIZE;
	unsigned long ROOT_INODE;
};

typedef struct superblock SuperBlock;

struct direntry
{
	unsigned long INODE_NUMBER;
	bool ISFILE;
	char ENTRY_NAME[16];
};

typedef struct direntry DirEntry;

struct dirheader
{
	unsigned short NUMBER;
	char NAME[16];
};

typedef struct dirheader DirHeader;

struct directory
{
	DirHeader HEADER;
	DirEntry* ENTRIES;
};

typedef struct directory Directory;