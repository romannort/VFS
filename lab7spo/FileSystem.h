#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

#include "INode.h"

void WriteToFS(std::stringstream& str, unsigned int cluster_size, unsigned long offset)
{
	std::fstream FSFile("testfile.bin", std::fstream::in | 
										std::fstream::out | 
										std::fstream::binary);

	FSFile.seekp(offset);

	char *buffer = new char[cluster_size];
	while( !str.eof() && str.good())
	{
		memset(buffer, 0, cluster_size);
		str.read( buffer, cluster_size);
		FSFile.write( reinterpret_cast<char*>(buffer), cluster_size);
	}
	FSFile.close();
}

void WriteSuperBlockToFS(SuperBlock& sBlock)
{
	std::stringstream str(std::stringstream::in | std::stringstream::out | std::stringstream::binary );
	str.write( reinterpret_cast<char*>(&sBlock), sizeof(sBlock));
	WriteToFS(str, sBlock.CLUSTER_SIZE, 0);
}

// inode number ->
void WriteDirectoryToFS(Directory& dir, unsigned int cluster_size, unsigned long offset)
{
	std::stringstream str(std::stringstream::in | std::stringstream::out);
	str.write( reinterpret_cast<char*>(&dir.HEADER), sizeof(dir.HEADER));
	for(int i = 0; i < dir.HEADER.NUMBER; i++)
		str.write(reinterpret_cast<char*>(&dir.ENTRIES[i]), sizeof(dir.ENTRIES[i]));

	WriteToFS(str, cluster_size, offset);
}

void WriteInodeToFS(Inode& node, unsigned int cluster_size, unsigned long offset)
{
	std::stringstream str(std::stringstream::in | std::stringstream::out);
	str.write( reinterpret_cast<char*>(&node), sizeof(node));
	WriteToFS(str, cluster_size, offset);
}

void WriteEmptyPlaces(unsigned long free_space)
{
	std::fstream FSFile("testfile.bin", std::fstream::out | 
										std::fstream::binary | 
										std::fstream::app);

	FSFile.seekp(FSFile.end);
	char *buffer = new char[free_space];
	memset(buffer, 0, free_space);
	FSFile.write(buffer, free_space);
	FSFile.close();
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

unsigned long FindNextFreeInode(unsigned long startingOffset = 0L)
{

	SuperBlock superBlock = ReadSuperBlock();
	std::fstream FSFile("testfile.bin", std::fstream::in | 
										std::fstream::binary );
	
	if ( !FSFile.is_open())
	{
		throw std::exception("Can't open FS file.");
	}
	startingOffset = startingOffset ? startingOffset : superBlock.INODE_TABLE_START + sizeof(Inode);
	unsigned long offset = 0L;
	FSFile.seekg(startingOffset);
	Inode inode = {0};
	while ( FSFile.tellg() <= (superBlock.DATA_TABLE_START - sizeof(Inode)) )
	{
		FSFile.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
		if(inode.direct_offsets[0] == 0L)
		{
			offset = (unsigned long)FSFile.tellg() - sizeof(Inode);
			break;
		}
	}
	FSFile.close();
	return offset;
}


unsigned long FindNextFreeBlock(unsigned long startingOffset = 0L)
{
	SuperBlock superBlock = ReadSuperBlock();
	std::fstream FSFile("testfile.bin", std::fstream::in | 
										std::fstream::binary );
	
	if ( !FSFile.is_open())
	{
		throw std::exception("Can't open FS file.");
	}
	startingOffset = startingOffset ? startingOffset : superBlock.DATA_TABLE_START;
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
	FSFile.close();
	return offset;
}


void CreateNewFS()
{
	SuperBlock sBlock = SuperBlock();

	std::cout << "Please enter max size of file system (MB): ";
	std::cin >> sBlock.MAX_SIZE;
	sBlock.MAX_SIZE *= 1048576;

	std::cout << "Please enter size of cluster (B): ";
	std::cin >> sBlock.CLUSTER_SIZE;
//	sBlock.CLUSTER_SIZE *= 1024;
	sBlock.INODE_TABLE_START = sizeof(SuperBlock);
//	std::cout << "Please enter start size of node table(kB): ";
//	std::cin >> sBlock.INODE_TABLE_SIZE;
	sBlock.INODE_TABLE_SIZE = (long)(sBlock.MAX_SIZE * 0.125);

	Inode NullNode = Inode();
	NullNode.indirect_inode = 0;

	sBlock.DATA_TABLE_START = sBlock.INODE_TABLE_SIZE + sBlock.INODE_TABLE_START;
	sBlock.FREE_SIZE = sBlock.MAX_SIZE - sBlock.DATA_TABLE_START;
	sBlock.ROOT_INODE = sBlock.INODE_TABLE_START + sizeof(Inode);

	WriteEmptyPlaces(sBlock.MAX_SIZE + (sBlock.MAX_SIZE - sBlock.DATA_TABLE_START)/sBlock.CLUSTER_SIZE );
	WriteSuperBlockToFS(sBlock);

	Directory rootDir = Directory();
	rootDir.HEADER = DirHeader();
	strcpy(rootDir.HEADER.NAME, "//");
	rootDir.HEADER.NUMBER = 0;

	WriteInodeToFS(NullNode, sBlock.CLUSTER_SIZE, sBlock.INODE_TABLE_START);
	WriteDirectoryToFS(rootDir, sBlock.CLUSTER_SIZE, sBlock.DATA_TABLE_START);

	// Dummy INode
	Inode inode = { {1,23,4,5,6,7,8,9,0,1,2}, 1000000L };
	WriteInodeToFS(inode, sBlock.CLUSTER_SIZE, sBlock.INODE_TABLE_START + sizeof(Inode));
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