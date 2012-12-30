#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <vector>

#include "INode.h"

unsigned long WriteDirectoryToFS(Directory& dir);
unsigned long WriteInode(Inode& inode, unsigned long inode_offset, std::fstream& file);
Inode& ReadInode(std::fstream& file, unsigned long offset);
unsigned long FindNextFreeInode(std::fstream& FSFile);
unsigned long FindNextFreeBlock(std::fstream& FSFile);
unsigned long GetDirByName(char* dirPath, Directory& dir, Inode& dirInode);
SuperBlock& ReadSuperBlock(std::fstream& file);
void UpdateDirectory(std::fstream& file, Directory& dir, Inode& dirInode);

unsigned long WriteToFS(std::stringstream& str)
{

	std::fstream FSFile("testfile.bin", std::fstream::in | 
										std::fstream::out | 
										std::fstream::binary);

	SuperBlock superBlock;
	FSFile.read(reinterpret_cast<char*>(&superBlock), sizeof(SuperBlock));

	char *buffer = new char[superBlock.CLUSTER_SIZE];
	std::vector<unsigned long> inode_numbers;
	inode_numbers.push_back( FindNextFreeInode(FSFile));
	Inode inode = ReadInode( FSFile, inode_numbers.back());
	unsigned long offset = 0UL;
	int current_inode_entry = 0;
	char one = '\x1';

	while( !str.eof() && str.good())
	{
		memset(buffer, 0, superBlock.CLUSTER_SIZE);
		str.read( buffer, superBlock.CLUSTER_SIZE);
		offset = FindNextFreeBlock(FSFile);
		inode.direct_offsets[current_inode_entry++] = offset;
		FSFile.seekp(offset);
		FSFile.write(reinterpret_cast<char*>(&one), 1); // non-free block
		FSFile.write(reinterpret_cast<char*>(buffer), superBlock.CLUSTER_SIZE);
		if ( current_inode_entry == 12)
		{
			inode_numbers.push_back( FindNextFreeInode(FSFile));
			inode.indirect_inode = inode_numbers.back();
			WriteInode(inode, inode_numbers.at( inode_numbers.size() - 2 ), FSFile);
			inode = ReadInode(FSFile, inode_numbers.back());
			current_inode_entry = 0;
		}
		FSFile.seekp(inode_numbers.back());
		FSFile.write(reinterpret_cast<char*>(&inode), sizeof(Inode));
	}
	FSFile.close();
	return inode_numbers.front();
}

Inode& WriteToDirectory(char* dirPath, DirEntry& newEntry)
{
	Inode dirInode;
	Directory dir;
	unsigned long dirOffset = GetDirByName(dirPath, dir, dirInode);
	dir.HEADER.NUMBER++;
	dir.ENTRIES[dir.HEADER.NUMBER-1] = newEntry; // hidden problems ?

	std::fstream file("testfile.bin", std::fstream::in | std::fstream::out | std::fstream::binary);
	if ( !file.is_open())
	{
		throw std::exception("Cant open FS file.");
	}
	UpdateDirectory(file, dir, dirInode);

	return dirInode;
}

Inode& ReadInode(std::fstream& file, unsigned long offset)
{
	file.seekg(offset);
	Inode inode;
	file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
	inode.own_offset = (unsigned long)file.tellp() - sizeof(Inode);
	return inode;
}

unsigned long WriteInode(Inode& inode, unsigned long inode_offset, std::fstream& file)
{
	unsigned long oldPutPointer = file.tellp();
	file.seekp(inode_offset);
	file.write(reinterpret_cast<char*>(&inode), sizeof(Inode));
	return oldPutPointer;
}

void WriteSuperBlockToFS(SuperBlock& sBlock)
{
	std::fstream file("testfile.bin", std::fstream::out | 
										std::fstream::binary | std::fstream::in );
	if ( !file.is_open())
	{
		throw std::exception("Can't open FS file");
	}
	file.write(reinterpret_cast<char*>(&sBlock), sizeof(SuperBlock));
	file.close();
}


unsigned long WriteDirectoryToFS(Directory& dir)
{
	std::stringstream str(std::stringstream::in | std::stringstream::out);
	str.write( reinterpret_cast<char*>(&dir.HEADER), sizeof(dir.HEADER));
	for(int i = 0; i < dir.HEADER.NUMBER; i++)
		str.write(reinterpret_cast<char*>(&dir.ENTRIES[i]), sizeof(dir.ENTRIES[i]));

	return WriteToFS(str);
}

//void WriteInodeToFS(Inode& node, unsigned int cluster_size, unsigned long offset)
//{
//	std::stringstream str(std::stringstream::in | std::stringstream::out);
//	str.write( reinterpret_cast<char*>(&node), sizeof(node));
//	WriteToFS(str);
//}

void WriteEmptyPlaces(unsigned long free_space, unsigned int cluster_size)
{
	std::fstream FSFile("testfile.bin", std::fstream::out | 
										std::fstream::binary);

	char* buffer = new char[cluster_size];
	memset(buffer, 0, cluster_size);
	while ( free_space != 0)
	{	
		FSFile.write(buffer, cluster_size);
		free_space -= cluster_size;
	}
	FSFile.close();
}

SuperBlock& ReadSuperBlock(std::fstream& file)
{
	SuperBlock superblock;
	file.read(reinterpret_cast<char*>(&superblock), sizeof(SuperBlock));
	return superblock;
}

SuperBlock& ReadSuperBlock()
{
	std::fstream FSFile("testfile.bin", std::fstream::in | std::fstream::binary);
	if ( !FSFile.is_open() )
	{
		throw std::exception("Cant open FS file.");
	}
	SuperBlock superblock;

	FSFile.read(reinterpret_cast<char*>(&superblock), sizeof(SuperBlock));
	FSFile.close();
	return superblock;
}

unsigned long FindNextFreeInode(std::fstream& FSFile)
{

	SuperBlock superBlock = ReadSuperBlock();
	/*std::fstream FSFile("testfile.bin", std::fstream::in | 
										std::fstream::binary );
	*/
	if ( !FSFile.is_open())
	{
		throw std::exception("Can't open FS file.");
	}
	unsigned long startingOffset = superBlock.INODE_TABLE_START + sizeof(Inode);
	unsigned long offset = 0L;
	FSFile.seekg(startingOffset);
	Inode inode = {0};
	while ( FSFile.tellg() <= (superBlock.DATA_TABLE_START - sizeof(Inode)) )
	{
		FSFile.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
		if(inode.used == 0)
		{
			offset = (unsigned long)FSFile.tellg() - sizeof(Inode);
			FSFile.seekp(offset);
			inode.used = 1;
			FSFile.write(reinterpret_cast<char*>(&inode), sizeof(Inode));
			break;
		}
	}
	//FSFile.close();
	return offset;
}

unsigned long FindNextFreeBlock(std::fstream& FSFile )
{
	SuperBlock superBlock = ReadSuperBlock();
	/*std::fstream FSFile("testfile.bin", std::fstream::in | 
										std::fstream::binary );
	*/
	if ( !FSFile.is_open())
	{
		throw std::exception("Can't open FS file.");
	}
	unsigned long startingOffset = superBlock.DATA_TABLE_START;
	unsigned long offset = 0L;
	FSFile.seekg(startingOffset);

	char *block = new char[superBlock.CLUSTER_SIZE+1];
	while ( FSFile.tellg() <= superBlock.MAX_SIZE - superBlock.CLUSTER_SIZE+1)
	{
		FSFile.read(block, superBlock.CLUSTER_SIZE+1);
		if ( block[0] == 0) // free block byte
		{
			offset = (unsigned long)FSFile.tellg() - superBlock.CLUSTER_SIZE - 1;
			break;
		}
	}
	//FSFile.close();
	return offset;
}

Directory& ReadDirectory(std::fstream& file, Inode& dirInode, SuperBlock& superBlock )
{
	char* buffer = new char[superBlock.CLUSTER_SIZE];
	std::stringstream stream( std::stringstream::in |
		std::stringstream::out |
		std::stringstream::binary);

	Inode tmpInode = dirInode;
	while(1)
	{
		for ( int i = 0; i < 12; ++i)
		{
			memset(buffer, 0, superBlock.CLUSTER_SIZE);
			if (tmpInode.direct_offsets[i] != 0)
			{
				file.seekg(tmpInode.direct_offsets[i] + 1); // ? add or not one for free block byte ?
				file.read(buffer, superBlock.CLUSTER_SIZE);
				stream.write(buffer, superBlock.CLUSTER_SIZE);
			}
			else
			{
				break;
			}
		}
		if (tmpInode.indirect_inode != 0)
		{
			file.seekg(tmpInode.indirect_inode);
			file.read(reinterpret_cast<char*>(&tmpInode), sizeof(Inode));
		}
		else
		{
			break;
		}
	}
	Directory dir;
	stream.read(reinterpret_cast<char*>(&dir.HEADER), sizeof(DirHeader));
	dir.ENTRIES = new DirEntry[dir.HEADER.NUMBER];
	for( int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		stream.read(reinterpret_cast<char*>(&dir.ENTRIES[i]), sizeof(DirEntry));
	}
	return dir;
}

unsigned long FindEntryInodeNumber(Directory& dir, const char* entryName)
{
	for ( int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		if(!strcmp(dir.ENTRIES[i].ENTRY_NAME, entryName))
		{
			return dir.ENTRIES[i].INODE_NUMBER;
		}
	}
	return -1;
}

std::vector<const char*> &split(char* s, char delim, std::vector<const char*>& elems) 
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) 
	{
		elems.push_back( item != "" ? item.c_str() : "/");
    }
	elems.push_back(0);
    return elems;
}

std::vector<const char*> split(char* s, char delim) {
    std::vector<const char*> elems;
    return split(s, delim, elems);
}


// dir - output arg: struct for dir with dirPath
// dirInode - output arg: first inode for dir with dirPath
unsigned long GetDirByName(char* dirPath, Directory& dir, Inode& dirInode)
{
	SuperBlock superBlock = ReadSuperBlock();

	std::fstream FSFile("testfile.bin", std::fstream::in |
		std::fstream::out | std::fstream::binary);

	if ( !FSFile.is_open())
	{
		throw std::exception("Can't open FS file.");
	}

	// Array of directories to go through
	std::vector<const char*> pathList = split(dirPath, '/');
	pathList.erase( pathList.begin()); 
	// Last entry in generated list - NULL

	Inode rootInode = {0};
	FSFile.seekg( superBlock.INODE_TABLE_START + sizeof(Inode));
	FSFile.read( reinterpret_cast<char*>(&rootInode), sizeof(Inode));
	Directory rootDir = ReadDirectory(FSFile, rootInode, superBlock);

	dir = rootDir;
	dirInode = rootInode;
	for ( int i = 0; pathList[i] != NULL; ++i)
	{
		unsigned long nextInodeNumber = FindEntryInodeNumber(dir, pathList[i]);
		if(nextInodeNumber == -1)
			return -1;
		FSFile.seekg(nextInodeNumber * sizeof(Inode));
		FSFile.read(reinterpret_cast<char*>(&dirInode), sizeof(Inode));
		dir = ReadDirectory(FSFile, dirInode, superBlock);
	}
	FSFile.close();
	return dirInode.direct_offsets[0]; // ???? unused
}



void UpdateDirectory(std::fstream& file, Directory& dir, Inode& dirInode)
{
	SuperBlock superBlock = ReadSuperBlock();
	char* buffer = new char[superBlock.CLUSTER_SIZE];
	std::stringstream stream( std::stringstream::in |
		std::stringstream::out |
		std::stringstream::binary);

	stream.write(reinterpret_cast<char*>(&dir.HEADER), sizeof(DirHeader));
	for( int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		stream.write(reinterpret_cast<char*>(&dir.ENTRIES[i]), sizeof(DirEntry));
	}
	// ------------
	Inode tmpInode = dirInode;

	while ( !stream.eof() && stream.good())
	{
		for ( int i = 0; i < 12; ++i)
		{
			memset(buffer, 0, superBlock.CLUSTER_SIZE);

			if (tmpInode.direct_offsets[i] == 0)
			{
				tmpInode.direct_offsets[i] = FindNextFreeBlock(file);
			}
			file.seekp(tmpInode.direct_offsets[i] + 1); // ? add or not one for free block byte ?
			stream.read(buffer, superBlock.CLUSTER_SIZE);
			file.write(buffer, superBlock.CLUSTER_SIZE);
			if  (stream.eof())
			{
				break;
			}
		}
		if  (stream.eof())
		{
			break;
		}

		if (tmpInode.indirect_inode != 0)
		{
			file.seekg(tmpInode.indirect_inode);
			file.read(reinterpret_cast<char*>(&tmpInode), sizeof(Inode));
		}
		else
		{
			unsigned long newNodeOffset = FindNextFreeInode(file);
			tmpInode.indirect_inode = newNodeOffset;
			WriteInode(tmpInode, tmpInode.own_offset, file);
			file.seekg(newNodeOffset);
			file.read(reinterpret_cast<char*>(&tmpInode), sizeof(Inode));
		}
	}
	//	WriteInode(tmpInode, tmpInode.own_offset, file); //?
}


void CreateNewFS()
{
	SuperBlock sBlock = SuperBlock();

	std::cout << "Please enter max size of file system (MB): ";
	std::cin >> sBlock.MAX_SIZE;
	sBlock.MAX_SIZE *= 1048576;

	std::cout << "Please enter size of cluster (B): ";
	std::cin >> sBlock.CLUSTER_SIZE;
	sBlock.INODE_TABLE_START = sizeof(SuperBlock);
	sBlock.INODE_TABLE_SIZE = (long)(sBlock.MAX_SIZE * 0.125);

	Inode NullNode = Inode();
	NullNode.indirect_inode = 0;
	NullNode.used = 1;

	sBlock.DATA_TABLE_START = sBlock.INODE_TABLE_SIZE + sBlock.INODE_TABLE_START;
	sBlock.FREE_SIZE = sBlock.MAX_SIZE - sBlock.DATA_TABLE_START;
	sBlock.ROOT_INODE = sBlock.INODE_TABLE_START + sizeof(Inode);

	WriteEmptyPlaces(sBlock.MAX_SIZE, sBlock.CLUSTER_SIZE  ); // + (sBlock.MAX_SIZE - sBlock.DATA_TABLE_START) / sBlock.CLUSTER_SIZE
	WriteSuperBlockToFS(sBlock);

	Directory rootDir = Directory();
	rootDir.HEADER = DirHeader();
	strcpy(rootDir.HEADER.NAME, "root");
	rootDir.HEADER.NUMBER = 0;

	
	// -----
	std::fstream file("testfile.bin", std::fstream::out | std::fstream::binary | std::fstream::in);
	file.seekp(sBlock.INODE_TABLE_START);
	NullNode.own_offset = file.tellp();
	file.write(reinterpret_cast<char*>(&NullNode), sizeof(Inode));
	file.close();
	
	WriteDirectoryToFS(rootDir);
}

void InitFS()
{
	std::fstream FSFile("testfile.bin", std::fstream::in | 
										std::fstream::binary | 
										std::fstream::out );
	if ( FSFile.is_open() != true)
	{
		CreateNewFS();
		return;
	}
	// check SB and FS consistency;
	FSFile.close();
}


std::vector<char*> ShowDirList(char* dirPath)
{
	Directory dir;
	Inode dirInode;
	GetDirByName(dirPath, dir, dirInode);

	std::vector<char*> dirList;
	for (int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		char* line = new char[80];
		memset(line, 0, 80);
		strcat(line, dir.ENTRIES[i].ENTRY_NAME);
		if ( !dir.ENTRIES[i].ISFILE ) 
		{
			strcat(line, "\t<DIR>");
		}
		dirList.push_back( line );
	}
	return dirList;
}

unsigned int GetNextINodeNumber()
{
	SuperBlock superBlock = ReadSuperBlock();
	Inode bufNode;

	std::fstream file("testfile.bin", std::fstream::out | std::fstream::binary | std::fstream::in);
	file.seekp(superBlock.INODE_TABLE_START);
	int nodenum = -1;
	do
	{
		file.read(reinterpret_cast<char*>(&bufNode), sizeof(Inode));
		nodenum++;
	}
	while(bufNode.used != 0);
	
	file.close();
	
	return nodenum;
}

Directory& AddDirectory(char* parentDirPath, char* dirName)
{
	Inode parentInode;
	Directory parentDir;
	GetDirByName(parentDirPath, parentDir, parentInode);

	Directory newDir;
	newDir.HEADER.NUMBER = 0;
	strcpy(newDir.HEADER.NAME, dirName);
	
	DirEntry newEntry;
	strcpy(newEntry.ENTRY_NAME, dirName);
	newEntry.ISFILE = 0;
//	newEntry.INODE_NUMBER = WriteDirectoryToFS(newDir); // ????? WTF
	newEntry.INODE_NUMBER = GetNextINodeNumber();
	

	if ( parentDir.HEADER.NUMBER == 0)
	{
		parentDir.ENTRIES = new DirEntry[1];
	}
	parentDir.HEADER.NUMBER++;
	parentDir.ENTRIES[parentDir.HEADER.NUMBER-1] = newEntry;
	
	std::fstream file("testfile.bin", std::fstream::out | std::fstream::binary | std::fstream::in );
	UpdateDirectory( file, parentDir, parentInode);
	file.close();
	return newDir;
}