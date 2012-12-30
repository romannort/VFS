#include <vector>
#include "FileSystem.h"

char* Commands[10] = {"help", "ls", "cd", "mkfile", "mkdir", "rm", "cp", "mv", "rename?", "exit"};

int GetCommandNum(char* command)
{
	for(int i = 0; i < 10; i++)
	{
		if(strstr(command, Commands[i]) == command)
		{
			return i;
		}
	}
	return -1;
}



void PrintHelp()							//1
{
	puts("\n\nUsage:");
	puts("\thelp - ");
	puts("\tls - ");
	puts("\tcd - ");
	puts("\tmkfile - ");
	puts("\tmkdir - ");
	puts("\trm - ");
	puts("\tcp - ");
	puts("\tmv - ");
	puts("\trename - ");
	puts("\texit - ");
}

void ShowDirectory(char* currentDir)						//2
{
	std::vector<char*> lst = ShowDirList(currentDir);
	printf("%d entries\n", lst.size());
	for ( int i = 0; i < lst.size(); ++i)
	{
		puts(lst[i]);
	}
}

char* ChangeDirectory(char* path)						//3
{
	Directory dir;
	Inode dirInode;
	if(GetDirByName(path, dir, dirInode) == -1)
	{
		std::cout << "No such directory!";
		return NULL;
	}
	else
	{
		return path;
	}
}

void AddNewFile()							//4
{
	puts("\n\tFile added\n");
}

void AddNewDir(char* currentDir, char* command)							//5
{
	std::cin >> command;
	AddDirectory(currentDir, command);
}

void Remove()								//6
{
	puts("\n\tRemoving done\n");
}

void Copy()									//7
{
	puts("\n\tCopying done\n");
}

void Move()									//8
{
	puts("\n\tMoving done\n");
}

void Rename()								//9
{
	puts("\n\tRenaming done\n");
}

void ExecuteCommand()
{
	char currentDir[100];
	char* path = new char[100];					
	strcpy(currentDir, "/root");
	char command[100];
	while( true )
	{
		printf("%s# ", currentDir);
		std::cin >> command;
		switch(GetCommandNum(command))
			{
				case 0:
					PrintHelp();
					break;
				case 1:
					ShowDirectory(currentDir);
					break;
				case 2:
					std::cin >> path;
					strcpy(command, ChangeDirectory(path));
					if(command != "")
						strcpy(currentDir, command);
					break;
				case 3:
					AddNewFile();
					break;
				case 4:
					AddNewDir(currentDir, command);
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
				default:
					printf("Unknown command '%s'. Use help to get a command list.\n", command);		
					break;
			}
	}
}