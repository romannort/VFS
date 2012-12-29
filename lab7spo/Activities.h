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
	return 0;
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

void ChangeDirectory(char* currentDir, char* command)						//3
{
	char* path = strtok( command, " \t");

}

void AddNewFile()							//4
{
	puts("\n\tFile added\n");
}

void AddNewDir(char* currentDir, char* command)							//5
{
	char* path = strtok(command , " \t");
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

void ExecuteCommand(char* currentDir)
{
	char command[100];
	scanf("%s", command);
	switch(GetCommandNum(command))
		{
			case 0:
				PrintHelp();
				break;
			case 1:
				ShowDirectory(currentDir);
				break;
			case 2:
				ChangeDirectory(currentDir, command);
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
		}
}