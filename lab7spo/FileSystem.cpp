#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include "Activities.h"
#include "FileSystem.h"

void main ( int argc, char** argv)
{
	InitFS();

	while( true )
	{
		printf(">>");
		switch(GetCommandNum())
		{
			case 0:
				PrintHelp();
				break;
			case 1:
				ShowDirectory();
				break;
			case 2:
				ChangeDirectory();
				break;
			case 3:
				AddNewFile();
				break;
			case 4:
				AddNewDir();
				break;
			case 5:
				Remove();
				break;
			case 6:
				Copy();
				break;
			case 7:
				Move();
				break;
			case 8:
				Rename();
				break;
			case 9:
				return;
		}
	}
/*	std::fstream testFile("testfile.bin", std::fstream::out | std::fstream::binary
		| std::fstream::trunc);

	std::stringstream str(std::stringstream::in | std::stringstream::out);

	Directory dir = Directory();

	if ( testFile.is_open() != true)
	{
		return EXIT_FAILURE;
	}

	dir.HEADER = DirHeader();
	dir.HEADER.NUMBER = 2;
	strcpy(dir.HEADER.NAME, "abcdefghijklmno");
	dir.ENTRIES = new DirEntry[dir.HEADER.NUMBER];
	dir.ENTRIES[0] = DirEntry();
	strcpy(dir.ENTRIES[0].ENTRY_NAME,"Entry1234567890");
	dir.ENTRIES[0].INODE_NUMBER = 4294967295L;
	dir.ENTRIES[1] = DirEntry();
	strcpy(dir.ENTRIES[1].ENTRY_NAME,"Entry2234567890");
	dir.ENTRIES[1].INODE_NUMBER = 0000000000L;

	str.write( reinterpret_cast<char*>(&dir.HEADER), sizeof(DirHeader));
	//str.write( reinterpret_cast<char*>(&dir.NUMBER), sizeof(dir.NUMBER));
	for ( int i = 0; i < dir.HEADER.NUMBER; ++i)
	{
		str.write( reinterpret_cast<char*>(&dir.ENTRIES[i]), sizeof(DirEntry));
	}
	
	char buffer[128] = {0};
	while( !str.eof() && str.good())
	{
		memset(buffer, 0, 128);
		str.read( buffer, 128);
		testFile.write( reinterpret_cast<char*>(buffer), 128);
	}
	testFile.close();
//--------------------------------------------------------------------------------------
	testFile.open("testfile.bin", std::fstream::in | std::fstream::binary);

	std::stringstream str1(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

	Directory dir1 = {0};

	if ( testFile.is_open() != true)
	{
		return EXIT_FAILURE;
	}

	while (testFile.good() && !testFile.eof())
	{
		memset(buffer, 0, 128);
		testFile.read( buffer, 128);
		str1.write(buffer, sizeof(buffer));
		std::cout.write( buffer, sizeof(buffer));
	}

	str1.read(reinterpret_cast<char*>(&dir1.HEADER), sizeof(DirHeader));

	dir1.ENTRIES = new DirEntry[dir1.HEADER.NUMBER];

	for(int i=0; i<dir1.HEADER.NUMBER; i++)
	{
		str1.read(reinterpret_cast<char*>(&dir1.ENTRIES[i]), sizeof(DirEntry));
	}

	testFile.close();
	
	std::cout << dir1.HEADER.NUMBER << '\n' << dir1.ENTRIES[0].ENTRY_NAME << ' ' << 	dir1.ENTRIES[0].INODE_NUMBER 
			 << '\n' << dir1.ENTRIES[1].ENTRY_NAME << ' ' << 	dir1.ENTRIES[1].INODE_NUMBER << '\n';*/
}