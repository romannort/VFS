#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <direct.h>
#include <dirent.h>

#include "INode.h"

unsigned long WriteDirectoryToFS(Directory& dir);
void WriteInode(Inode& inode, unsigned long inode_offset, std::fstream& file);
Inode& ReadInode(std::fstream& file, unsigned long offset);
unsigned long FindNextFreeInode(std::fstream& FSFile);
unsigned long FindNextFreeBlock(std::fstream& FSFile);
unsigned long GetDirByName(std::string dirPath, Directory& dir, Inode& dirInode);
SuperBlock& ReadSuperBlock(std::fstream& file);
SuperBlock& ReadSuperBlock();
void UpdateDirectory(std::fstream& file, Directory& dir, Inode& dirInode);
void AddFile (std::string dirPath, const char* fileName, std::stringstream& fileData);
void AddNewFile(std::string currentDir, std::string& fileName, std::string& externFileName);
unsigned long ReadFile (std::string dirPath, std::string fileName, std::stringstream& fileStr);
unsigned long ReadFile (Directory dir, char* fileName, std::stringstream& fileStr);
// -----------------------------------------------------------------------

void WriteDataBlock(char* data, SuperBlock& sb, unsigned long data_block_number, std::fstream& FSFile, int free = 0)
{
	char one = '\x1';
	char null = '\0';
	unsigned long offset = data_block_number * sb.CLUSTER_SIZE + sb.DATA_TABLE_START;
	FSFile.seekp(offset);
	if ( free == 1)
	{
		FSFile.write(reinterpret_cast<char*>(&null), 1); // free block
	}
	else
	{
		FSFile.write(reinterpret_cast<char*>(&one), 1); // non-free block
	}
	FSFile.write(reinterpret_cast<char*>(data), sb.CLUSTER_SIZE - 1);
}

unsigned long WriteToFS(std::stringstream& str)
{
	std::fstream FSFile("testfile.bin", std::fstream::in | 
		std::fstream::out | 
		std::fstream::binary);

	SuperBlock superBlock;
	FSFile.read(reinterpret_cast<char*>(&superBlock), sizeof(SuperBlock));

	char *buffer = new char[superBlock.CLUSTER_SIZE - 1];
	std::vector<unsigned long> inode_numbers;
	inode_numbers.push_back( FindNextFreeInode(FSFile)); // 0 - no free inodes
	Inode inode = ReadInode( FSFile, inode_numbers.back());
	unsigned long datablock_number = 0UL;
	int current_inode_entry = 0;

	while( !str.eof() && str.good())
	{
		memset(buffer, 0, superBlock.CLUSTER_SIZE - 1);
		str.read( buffer, superBlock.CLUSTER_SIZE - 1);
		datablock_number = FindNextFreeBlock(FSFile);
		inode.DATA_NUMBERS[current_inode_entry++] = datablock_number;

		WriteDataBlock(buffer, superBlock, datablock_number, FSFile);
		if ( current_inode_entry == INODE_DIRECT_OFFSETS)
		{
			inode_numbers.push_back( FindNextFreeInode(FSFile));
			inode.indirect_inode = inode_numbers.back();
			WriteInode(inode, inode_numbers.at( inode_numbers.size() - 2 ), FSFile);
			inode = ReadInode(FSFile, inode_numbers.back());
			current_inode_entry = 0;
		}
		WriteInode(inode, inode.NUMBER, FSFile);
	}
	FSFile.close();

	delete[] buffer;
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
	file.close();
	return dirInode;
}

// 
Inode& ReadInode(std::fstream& file, unsigned long number)
{
	unsigned long offset = number * sizeof(Inode) + sizeof(SuperBlock);
	file.seekg(offset);
	Inode inode;
	file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
	inode.NUMBER = number;
	return inode;
}

void WriteInode(Inode& inode, unsigned long inode_number, std::fstream& file)
{
	unsigned long inode_offset = inode_number * sizeof(Inode) + sizeof(SuperBlock);
	file.seekp(inode_offset);
	file.write(reinterpret_cast<char*>(&inode), sizeof(Inode));

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
	{
		str.write(reinterpret_cast<char*>(&dir.ENTRIES[i]), sizeof(dir.ENTRIES[i]));
	}
	return WriteToFS(str);
}

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

SuperBlock& ReadSuperBlock()
{
	std::fstream FSFile("testfile.bin", std::fstream::in | std::fstream::binary);
	if ( !FSFile.is_open() )
	{
		throw std::exception("Cant open FS file.");
	}
	SuperBlock superblock = ReadSuperBlock(FSFile);
	FSFile.close();
	return superblock;
}

SuperBlock& ReadSuperBlock(std::fstream& file)
{
	SuperBlock superblock;
	file.seekg(0);
	file.read(reinterpret_cast<char*>(&superblock), sizeof(SuperBlock));
	return superblock;
}

unsigned long CalculateInodeNumber(unsigned long offset)
{
	return (offset - sizeof(SuperBlock) ) / sizeof(Inode);
}

// return NUmber of Inode
unsigned long FindNextFreeInode(std::fstream& FSFile)
{

	SuperBlock superBlock = ReadSuperBlock(FSFile);
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
			inode.NUMBER = CalculateInodeNumber(offset);
			FSFile.write(reinterpret_cast<char*>(&inode), sizeof(Inode));
			break;
		}
	}
	return CalculateInodeNumber(offset);
}

// return number of block 
// number * cluster_size == block offset
unsigned long FindNextFreeBlock(std::fstream& FSFile )
{
	SuperBlock superBlock = ReadSuperBlock(FSFile);
	if ( !FSFile.is_open())
	{
		throw std::exception("Can't open FS file.");
	}
	unsigned long startingOffset = superBlock.DATA_TABLE_START + superBlock.CLUSTER_SIZE;
	unsigned long offset = 0L;
	FSFile.seekg(startingOffset);
	char *block = new char[superBlock.CLUSTER_SIZE];
	while ( FSFile.tellg() <= superBlock.MAX_SIZE - superBlock.CLUSTER_SIZE)
	{
		FSFile.read(block, superBlock.CLUSTER_SIZE);
		if ( block[0] == 0) // free block byte
		{
			offset = (unsigned long)FSFile.tellg() - superBlock.CLUSTER_SIZE;
			break;
		}
	}
	return (offset - superBlock.DATA_TABLE_START) / superBlock.CLUSTER_SIZE;
}

char* ReadDataBlock(char* data, unsigned long block_number, SuperBlock& sb, std::fstream& file)
{
	memset(data, 0, sb.CLUSTER_SIZE - 1 );
	unsigned long offset = block_number * (sb.CLUSTER_SIZE)  + sb.DATA_TABLE_START + 1;
	file.seekg(offset); // ? add or not one for free block byte ?
	file.read(data, sb.CLUSTER_SIZE - 1);
	return data;
}



Directory& ReadDirectory(std::fstream& file, Inode& dirInode, SuperBlock& superBlock )
{
	char* buffer = new char[superBlock.CLUSTER_SIZE - 1];
	std::stringstream stream( std::stringstream::in |
		std::stringstream::out |
		std::stringstream::binary);

	Inode tmpInode = dirInode;
	while(1)
	{
		for ( int i = 0; i < INODE_DIRECT_OFFSETS; ++i)
		{
			if (tmpInode.DATA_NUMBERS[i] != 0)
			{
				ReadDataBlock(buffer, tmpInode.DATA_NUMBERS[i], superBlock, file);
				stream.write(buffer, superBlock.CLUSTER_SIZE - 1);
			}
			else
			{
				break;
			}
		}
		if (tmpInode.indirect_inode != 0)
		{

			tmpInode = ReadInode(file, tmpInode.indirect_inode);
		}
		else
		{
			break;
		}
	}
	Directory dir;
	stream.read(reinterpret_cast<char*>(&dir.HEADER), sizeof(DirHeader));
	dir.ENTRIES = new DirEntry[dir.HEADER.NUMBER + 1];
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
	return 0; // 0 - NULL_INODE
}

// split line by delimeter
// last entry - emprt string
std::vector<std::string> &split(char* s, char delim, std::vector<std::string>& elems) 
{
	std::stringstream ss(s);
	std::string item;
	while(std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	elems.push_back("");
	return elems;
}
std::vector<std::string> split(char* s, char delim) {
	std::vector<std::string> elems;
	return split(s, delim, elems);
}
std::vector<std::string> split(std::string s, char delim) {
	return split( (char*)s.c_str(), delim);
}

// dir - output arg: struct for dir with dirPath
// dirInode - output arg: first inode for dir with dirPath
unsigned long GetDirByName(std::string dirPath, Directory& dir, Inode& dirInode)
{
	SuperBlock superBlock = ReadSuperBlock();

	std::fstream FSFile("testfile.bin", std::fstream::in |
		std::fstream::out | std::fstream::binary);

	if ( !FSFile.is_open())
	{
		throw std::exception("Can't open FS file.");
	}

	// Array of directories to go through
	std::vector<std::string> pathList = split(dirPath, '/');
	pathList.erase( pathList.begin()); 
	// Last entry in generated list - NULL
	Inode rootInode = {0};
	FSFile.seekg( superBlock.INODE_TABLE_START + sizeof(Inode));
	FSFile.read( reinterpret_cast<char*>(&rootInode), sizeof(Inode));
	Directory rootDir = ReadDirectory(FSFile, rootInode, superBlock);

	dir = rootDir;
	dirInode = rootInode;
	for ( int i = 0; pathList[i] != ""; ++i)
	{
		unsigned long nextInodeNumber = FindEntryInodeNumber(dir, (char*)pathList[i].c_str());
		if(nextInodeNumber == 0) 
		{
			std::cout << "No such directory!";
			FSFile.close();
			return -1;
		}
		dirInode = ReadInode(FSFile, nextInodeNumber);
		//FSFile.seekg(nextInodeNumber * (superBlock.CLUSTER_SIZE)  + superBlock.INODE_TABLE_START); //?
		//FSFile.read(reinterpret_cast<char*>(&dirInode), sizeof(Inode));
		dir = ReadDirectory(FSFile, dirInode, superBlock);
	}
	FSFile.close();
	return dirInode.DATA_NUMBERS[0]; // ???? unused
}


void FreeUnusedNodes(Inode& node, SuperBlock& superBlock, std::fstream& file)
{
	char *buffer = new char[superBlock.CLUSTER_SIZE];
	memset(buffer, 0, superBlock.CLUSTER_SIZE);
	Inode tmpInode = node;
	while( 1 )
	{
		for ( int i = 0; i < INODE_DIRECT_OFFSETS; ++i)
		{
			if (tmpInode.DATA_NUMBERS[i] == 0)
			{
				break;
			}
			WriteDataBlock(buffer, superBlock, tmpInode.DATA_NUMBERS[i], file, 1);
		}
		if ( tmpInode.indirect_inode != 0 )
		{
			WriteInode(NULL_INODE, tmpInode.NUMBER, file);
			tmpInode = ReadInode(file, tmpInode.indirect_inode);
		}
		else
		{
			break;
		}
	}
}

void UpdateDirectory(std::fstream& file, Directory& dir, Inode& dirInode)
{
	SuperBlock superBlock = ReadSuperBlock(file);
	char* buffer = new char[superBlock.CLUSTER_SIZE - 1];
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
		for ( int i = 0; i < INODE_DIRECT_OFFSETS; ++i)
		{
			memset(buffer, 0, superBlock.CLUSTER_SIZE - 1);
			if (tmpInode.DATA_NUMBERS[i] == 0)
			{
				tmpInode.DATA_NUMBERS[i] = FindNextFreeBlock(file);
			}
			stream.read(buffer, superBlock.CLUSTER_SIZE - 1);
			WriteDataBlock(buffer, superBlock, tmpInode.DATA_NUMBERS[i], file);
			if  (stream.eof())
			{
				memset(buffer, 0, superBlock.CLUSTER_SIZE - 1);	
				for ( int j = i+1; j < INODE_DIRECT_OFFSETS; ++j)
				{
					if ( tmpInode.DATA_NUMBERS[j] != 0)
					{
						WriteDataBlock(buffer, superBlock, tmpInode.DATA_NUMBERS[j], file, 1); // free unused blocks in that inode
						tmpInode.DATA_NUMBERS[j] = 0;
					}
				}
				break;
			}
		}
		if  (stream.eof())
		{
			if (tmpInode.indirect_inode != 0)
			{
				FreeUnusedNodes(tmpInode, superBlock, file);
			}
			break;
		}
		if (tmpInode.indirect_inode != 0)
		{
			tmpInode = ReadInode(file, tmpInode.indirect_inode);
		}
		else
		{
			unsigned long newNodeNumber = FindNextFreeInode(file);
			tmpInode.indirect_inode = newNodeNumber;
			WriteInode(tmpInode, tmpInode.NUMBER, file);
			tmpInode = ReadInode(file, newNodeNumber);
		}
	}
	WriteInode(tmpInode, tmpInode.NUMBER, file); //?
}

void MoveHadler(std::string sourceParent, std::string source, std::string dest)
{
	Directory sourceDir, destDir;
	Inode sourceInode, destInode;
	GetDirByName(sourceParent, sourceDir, sourceInode);
	GetDirByName(dest, destDir, destInode);
	DirEntry toMove;

	for ( int i = 0; i < sourceDir.HEADER.NUMBER; ++i)
	{
		if(!strcmp(sourceDir.ENTRIES[i].ENTRY_NAME, source.c_str()))
		{
			toMove = sourceDir.ENTRIES[i];
			for ( int j = i+1; j < sourceDir.HEADER.NUMBER; ++j)
			{
				sourceDir.ENTRIES[i++] = sourceDir.ENTRIES[j];
			}
		}
	}
	sourceDir.HEADER.NUMBER--;
	destDir.HEADER.NUMBER++;
	destDir.ENTRIES[destDir.HEADER.NUMBER-1] = toMove;

	std::fstream file("testfile.bin", std::fstream::in | 
		std::fstream::binary | 
		std::fstream::out );
	UpdateDirectory(file, sourceDir, sourceInode);
	UpdateDirectory(file, destDir, destInode);
	file.close();
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
	strcpy(rootDir.HEADER.NAME, "/");
	rootDir.HEADER.NUMBER = 0;

	// ----- Write NullNode
	std::fstream file("testfile.bin", std::fstream::out | std::fstream::binary | std::fstream::in);
	file.seekp(sBlock.INODE_TABLE_START);
	NullNode.NUMBER = 0;
	file.write(reinterpret_cast<char*>(&NullNode), sizeof(Inode));
	file.close();

	WriteDirectoryToFS(rootDir);
}

bool NameIsValid(std::string path, std::string name, bool isFile)
{
	for(int i = 0; i < name.length(); i++)
		if((name[i] > '9' || name[i] < '0') &&
			(name[i] > 'Z' || name[i] < 'A') &&	
			(name[i] > 'z' || name[i] < 'a'))
		{
			std::cout << "Directory name must not contain any characters except A-Z, a-z, 0-9!\n\n";
			return false;
		}
		Directory dir;
		Inode dirInode;
		GetDirByName(path, dir, dirInode);
		for ( int i = 0; i < dir.HEADER.NUMBER; ++i)
		{
			if(!strcmp(dir.ENTRIES[i].ENTRY_NAME, name.c_str()))
			{
				std::cout << "Such directory is already exist!\n\n";
				return false;
			}
		}
		return true;
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

std::vector<std::string> ShowDirList(std::string dirPath)
{
	Directory dir;
	Inode dirInode;
	GetDirByName(dirPath, dir, dirInode);

	std::vector<std::string> dirList;
	for (int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		std::string line(50, ' ');
		line.insert( 0,dir.ENTRIES[i].ENTRY_NAME);
		if ( !dir.ENTRIES[i].ISFILE )
		{
			line.insert(30," <DIR>");
		}
		dirList.push_back( line );
	}
	return dirList;
}

Directory& AddDirectory(std::string parentDirPath, const char* dirName)
{
	Inode parentInode;
	Directory parentDir;
	GetDirByName(parentDirPath, parentDir, parentInode);
	Directory newDir;
	newDir.HEADER.NUMBER = 0;
	memset(newDir.HEADER.NAME, 0, 16);
	strcpy(newDir.HEADER.NAME, dirName);
	DirEntry newEntry;
	memset(newEntry.ENTRY_NAME, 0, 16);
	strcpy(newEntry.ENTRY_NAME, dirName);
	newEntry.ISFILE = 0;
	newEntry.INODE_NUMBER = WriteDirectoryToFS(newDir);
	parentDir.HEADER.NUMBER++;
	parentDir.ENTRIES[parentDir.HEADER.NUMBER-1] = newEntry;
	std::fstream file("testfile.bin", std::fstream::out | std::fstream::binary | std::fstream::in );
	UpdateDirectory( file, parentDir, parentInode);
	file.close();
	delete [] parentDir.ENTRIES;
	return newDir;
}

void UnReadDataBlock(std::fstream& file, unsigned long datablock_number, SuperBlock& sb)
{
	char* buffer = new char[sb.CLUSTER_SIZE];
	memset(buffer, 0, sb.CLUSTER_SIZE);
	WriteDataBlock(buffer, sb, datablock_number, file, 1);
}

void RemoveEntry(Inode& inode, std::fstream& file, SuperBlock& sb)
{
	Inode tmpInode = inode;
	while(1)
	{
		for ( int i = 0; i < INODE_DIRECT_OFFSETS; ++i)
		{
			if (tmpInode.DATA_NUMBERS[i] != 0)
			{
				UnReadDataBlock( file, tmpInode.DATA_NUMBERS[i], sb);
			}
			else
			{
				break;
			}
		}
		if (tmpInode.indirect_inode != 0)
		{
			unsigned long number = tmpInode.NUMBER;
			tmpInode = ReadInode(file, tmpInode.indirect_inode);
			WriteInode(NULL_INODE, number, file);
		}
		else
		{
			break;
		}
	}
	WriteInode(NULL_INODE, tmpInode.NUMBER, file);
}

void RemoveDir(Directory& dir, Inode& dirInode, SuperBlock& sb, std::fstream& file)
{
	Inode tmpInode = dirInode;

	for ( int j = 0; j < dir.HEADER.NUMBER; ++j)
	{
		if (dir.ENTRIES[j].ISFILE == false)
		{
			Inode childInode = ReadInode(file, dir.ENTRIES[j].INODE_NUMBER);
			Directory childDir = ReadDirectory(file, childInode, sb);
			RemoveDir(childDir, childInode, sb, file);
		}
		else
		{
			Inode fileInode = ReadInode(file, dir.ENTRIES[j].INODE_NUMBER);
			RemoveEntry(fileInode, file, sb);
		}
	}
	RemoveEntry( dirInode, file, sb);
}


int RemoveDir(std::string& parentPath, std::string& target)
{
	std::string targetPath = parentPath + ( parentPath.back() == '/' ? "" : "/")  + target;

	Directory parentDir;
	Inode parentInode;
	GetDirByName( parentPath, parentDir, parentInode);

	if ( FindEntryInodeNumber(parentDir, target.c_str())  == 0)
	{
		return 1;
	}
	Directory targetDir;
	Inode targetInode;
	GetDirByName( targetPath, targetDir, targetInode);


	for ( int i = 0; i < parentDir.HEADER.NUMBER; ++i)
	{
		if(!strcmp(parentDir.ENTRIES[i].ENTRY_NAME, target.c_str()))
		{
			for ( int j = i+1; j < parentDir.HEADER.NUMBER; ++j)
			{
				parentDir.ENTRIES[i++] = parentDir.ENTRIES[j];
			}
		}
	}
	parentDir.HEADER.NUMBER--;

	std::fstream file("testfile.bin", std::fstream::in | 
		std::fstream::binary | 
		std::fstream::out );
	SuperBlock sb = ReadSuperBlock(file);
	RemoveDir(targetDir, targetInode, sb, file);
	UpdateDirectory(file, parentDir, parentInode);
	file.close();
	return 0;
}


void RemoveFile(std::string& filePath, std::string& parentPath)
{

	std::string parentDirPath = parentPath;
	std::string fileName = filePath;

	Directory parentDir;
	Inode parentInode;
	GetDirByName( parentDirPath, parentDir, parentInode);
	Inode fileInode;

	std::fstream file("testfile.bin", std::fstream::in | 
		std::fstream::binary | 
		std::fstream::out );
	for ( int i = 0; i < parentDir.HEADER.NUMBER; ++i)
	{
		if(!strcmp(parentDir.ENTRIES[i].ENTRY_NAME, fileName.c_str()))
		{
			fileInode = ReadInode(file, parentDir.ENTRIES[i].INODE_NUMBER);
			for ( int j = i+1; j < parentDir.HEADER.NUMBER; ++j)
			{
				parentDir.ENTRIES[i++] = parentDir.ENTRIES[j];
			}
		}
	}
	parentDir.HEADER.NUMBER--;

	SuperBlock sb = ReadSuperBlock(file);
	RemoveEntry(fileInode, file, sb);
	UpdateDirectory(file, parentDir, parentInode);
	file.close();
}

// recursive copy dir tree to extern disk
void RecursiveCopyOut(std::string& externPath, Inode& inode, Directory& dir, std::fstream& file, SuperBlock& sb)
{
	mkdir(externPath.c_str());
	std::stringstream fileStr(std::stringstream::in | std::stringstream::out);
	for ( int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		Inode tmpInode = ReadInode(file, dir.ENTRIES[i].INODE_NUMBER);
		if ( dir.ENTRIES[i].ISFILE ==  0 ) // dir
		{
			Directory tmpDir = ReadDirectory( file, tmpInode, sb);
			RecursiveCopyOut( externPath + "\\" + dir.ENTRIES[i].ENTRY_NAME,  tmpInode, tmpDir, file ,sb);
		}
		else
		{
			if (ReadFile(dir, dir.ENTRIES[i].ENTRY_NAME, fileStr) != -1)
			{
				std::fstream file((externPath+"\\"+dir.ENTRIES[i].ENTRY_NAME).c_str(), std::fstream::out | std::fstream::binary );
				std::copy(std::istreambuf_iterator<char>(fileStr),
						std::istreambuf_iterator<char>(),
						std::ostreambuf_iterator<char>(file));
				file.close();
			}
		}
	}
}

int CopyOutDirectories(std::string& dirToCopy,  std::string& parentPath, std::string& externPath)
{
	std::string targetPath = parentPath + ( parentPath.back() == '/' ? "" : "/")  + dirToCopy;
	Directory parentDir;
	Inode parentInode;
	GetDirByName( parentPath, parentDir, parentInode);

	if ( FindEntryInodeNumber(parentDir, dirToCopy.c_str())  == 0)
	{
		return 1;
	}
	Directory dir;
	Inode  tocopyInode;
	GetDirByName( targetPath, dir, tocopyInode);

	std::fstream file("testfile.bin", std::fstream::in | 
		std::fstream::binary | 
		std::fstream::out );
	SuperBlock sb = ReadSuperBlock(file);
	externPath = externPath + ( externPath.back() == '\\' ? "" : "\\")  + dirToCopy;
	RecursiveCopyOut(externPath, tocopyInode, dir, file, sb);
}

std::stringstream& ReadFileToStream( std::fstream& file)
{
	std::stringstream str ( std::stringstream::in | std::stringstream::out );
	char* buffer = new char[512];

	while ( !file.eof())
	{
		memset(buffer, 0, 512);
		file.read(buffer, 512);
		str.write( buffer, 512);
	}
	delete [] buffer;
	return str;
}

void RecursiveCopyIn(std::string& outPath, std::string localPath )
{

	DIR *dirstruct = opendir(outPath.c_str());
	struct dirent *entry = readdir(dirstruct);
	while (entry != NULL)
	{
		if (entry->d_type == DT_DIR)
		{
			if (strcmp(entry->d_name, "..") && strcmp(entry->d_name, "."))
			{
				Directory subdir =  AddDirectory(localPath, entry->d_name);
				RecursiveCopyIn( outPath + "\\" + entry->d_name, localPath + "/" + subdir.HEADER.NAME);
			}
		}
		else
		{
			AddNewFile( localPath, std::string(entry->d_name), outPath + "\\" + entry->d_name);
		}
		entry = readdir(dirstruct);
	}
	closedir(dirstruct);
}

int CopyInDirectories(std::string& externDirPath, std::string& localDirPath, std::string& localParentPath)
{
	Directory parentDir;
	Inode parentInode;
	GetDirByName( localParentPath, parentDir, parentInode);

	// list all entries for existed folder localdirpath
	Directory newLocalDir;
	newLocalDir.HEADER.NUMBER = 0;
	memset(newLocalDir.HEADER.NAME, 0, 16);
	strcpy(newLocalDir.HEADER.NAME, localDirPath.c_str());
	DirEntry newEntry;
	memset(newEntry.ENTRY_NAME, 0, 16);
	strcpy(newEntry.ENTRY_NAME, localDirPath.c_str());
	newEntry.ISFILE = 0;
	newEntry.INODE_NUMBER = WriteDirectoryToFS(newLocalDir);
	parentDir.ENTRIES[parentDir.HEADER.NUMBER++] = newEntry;
	std::fstream file("testfile.bin", std::fstream::out | std::fstream::binary | std::fstream::in );
	UpdateDirectory(file, parentDir, parentInode);
	SuperBlock sb = ReadSuperBlock(file);
	// -=-==========
	Inode  inode = ReadInode(file, newEntry.INODE_NUMBER);
	RecursiveCopyIn(externDirPath, /*file, sb, inode, newLocalDir,*/ localParentPath  + 
		( localParentPath.back() == '/' ? "" : "/") + localDirPath);
	file.close();
	// ---------------
	return 0;
}

void AddFile (std::string dirPath, const char* fileName, std::stringstream& fileData)
{
	Inode fileInode, dirInode;
	DirEntry newEntry;
	Directory dir;
	GetDirByName(dirPath, dir, dirInode);



	strcpy(newEntry.ENTRY_NAME, fileName);
	newEntry.ISFILE = 1;
	fileData.seekg(0, std::ios_base::end);
	newEntry.FILE_SIZE = fileData.tellg(); ///
	fileData.seekg(0, std::ios_base::beg);
	newEntry.INODE_NUMBER = WriteToFS(fileData);
	dir.HEADER.NUMBER++;
	dir.ENTRIES[dir.HEADER.NUMBER-1] = newEntry;
	std::fstream file("testfile.bin", std::fstream::out | std::fstream::binary | std::fstream::in );
	UpdateDirectory( file, dir, dirInode);
	file.close();
	delete [] dir.ENTRIES;
}

void AddNewFile(std::string currentDir, std::string& fileName, std::string& externFileName)
{

	std::fstream file(externFileName.c_str(), std::fstream::out | std::fstream::binary | std::fstream::in );    
	std::stringstream fileStr(std::stringstream::in | std::stringstream::out);
	copy(std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>(),
		std::ostreambuf_iterator<char>(fileStr));
	AddFile(currentDir, fileName.c_str(), fileStr);                                                                     //
	file.close();
}

//void RecursiveCopyInside(std::string& toPath, Inode& inode, Directory& dir, std::fstream& file, SuperBlock& sb)
//{
//	//mkdir(externPath.c_str());
//
//	Directory copydir = AddDirectory(toPath, 
//
//	for ( int i = 0; i < dir.HEADER.NUMBER; ++i)
//	{
//		Inode tmpInode = ReadInode(file, dir.ENTRIES[i].INODE_NUMBER);
//		if ( dir.ENTRIES[i].ISFILE ==  0 ) // dir
//		{
//			Directory tmpDir = ReadDirectory( file, tmpInode, sb);
//			RecursiveCopyOut( externPath + "\\" + dir.ENTRIES[i].ENTRY_NAME,  tmpInode, tmpDir, file ,sb);
//		}
//		else
//		{
//			// copy file 
//		}
//	}
//}
//
//int CopyDirectoriesInside(std::string& dirToCopy,  std::string& parentPath, std::string& toPath)
//{
//	
//	Directory parentDir;
//	Inode parentInode;
//	GetDirByName( parentPath, parentDir, parentInode);
//
//	if ( FindEntryInodeNumber(parentDir, dirToCopy.c_str())  == 0)
//	{
//		return 1;
//	}
//	Directory dir;
//	Inode  tocopyInode;
//	GetDirByName( targetPath, dir, tocopyInode);
//
//	std::fstream file("testfile.bin", std::fstream::in | 
//		std::fstream::binary | 
//		std::fstream::out );
//	SuperBlock sb = ReadSuperBlock(file);
//	externPath = externPath + ( externPath.back() == '\\' ? "" : "\\")  + dirToCopy;
//	RecursiveCopyOut(externPath, tocopyInode, dir, file, sb);
//}

void ReadFromFS (unsigned long inode_number, unsigned long size, std::stringstream& fileStr)
{
	std::fstream FSFile("testfile.bin", std::fstream::in | 
		std::fstream::out | 
		std::fstream::binary);

	SuperBlock superBlock = ReadSuperBlock(FSFile);

	char *buffer = new char[superBlock.CLUSTER_SIZE - 1];
	Inode inode = ReadInode (FSFile, inode_number);
	unsigned long current_node_entry = 0;
	while (inode.DATA_NUMBERS[current_node_entry] != 0)
	{
		memset(buffer, 0, superBlock.CLUSTER_SIZE - 1);
		ReadDataBlock(buffer, inode.DATA_NUMBERS[current_node_entry], superBlock, FSFile);
		if (size > superBlock.CLUSTER_SIZE - 1)
			fileStr.write(buffer, superBlock.CLUSTER_SIZE - 1);
		else
		{
			fileStr.write(buffer, size);
			return;
		}
		size -= superBlock.CLUSTER_SIZE - 1;
		current_node_entry ++;
		if (current_node_entry == INODE_DIRECT_OFFSETS)
		{
			inode = ReadInode (FSFile, inode.indirect_inode);
			current_node_entry = 0;
		}
	}
	return;
}

unsigned long ReadFile (Directory dir, char* fileName, std::stringstream& fileStr)
{
	std::stringstream tempStr(std::stringstream::in | std::stringstream::out);
	for (int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		if (!strcmp(fileName, dir.ENTRIES[i].ENTRY_NAME))
		{
			ReadFromFS(dir.ENTRIES[i].INODE_NUMBER, dir.ENTRIES[i].FILE_SIZE, fileStr);
			return dir.ENTRIES[i].FILE_SIZE;
		}
	}
	return -1;
}

unsigned long ReadFile (std::string dirPath, std::string fileName, std::stringstream& fileStr)
{
	Directory dir;
	Inode dirInode;
	GetDirByName (dirPath, dir, dirInode);
	std::stringstream tempStr(std::stringstream::in | std::stringstream::out);
	for (int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		if (!strcmp(fileName.c_str(), dir.ENTRIES[i].ENTRY_NAME))
		{
			ReadFromFS(dir.ENTRIES[i].INODE_NUMBER, dir.ENTRIES[i].FILE_SIZE, fileStr);

			return dir.ENTRIES[i].FILE_SIZE;
		}
	}
	return -1;
}

void CopyFromFS(std::string currentDir,  std::vector<std::string> argv)
{
	std::stringstream fileStr(std::stringstream::in | std::stringstream::out);
	char buf[10] = {0};
	if ( ReadFile(currentDir, argv[1], fileStr) != -1)
	{

		std::fstream file((argv[2]+"\\"+argv[1]).c_str(), std::fstream::out | std::fstream::binary );
		std::copy(std::istreambuf_iterator<char>(fileStr),

			std::istreambuf_iterator<char>(),

			std::ostreambuf_iterator<char>(file));

		file.close();

	}
}