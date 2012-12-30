#include <vector>
#include "FileSystem.h"

char* Commands[10] = {"help", "ls", "cd", "mkfile", "mkdir", "rm", "cp", "mv", "rename?", "exit"};

int GetCommandNum(std::string command)
{
	for(int i = 0; i < 10; i++)
	{
		if(strstr(command.c_str() , Commands[i]) == command.c_str())
		{
			return i;
		}
	}
	return 0;
}

//std::vector<const char*> ParseCommand()
//{
//	std::vector<std::string> argv;
//	while ( !std::cin.end )
//	{
//		std::string commandPart;
//		std::cin >> commandPart;
//		argv.push_back(commandPart);
//	}
//	return argv;
//}

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
	std::vector<std::string> lst = ShowDirList(currentDir);
	printf("%d entries\n", lst.size());
	for ( int i = 0; i < lst.size(); ++i)
	{
		std::cout << lst[i] << std::endl;
	}
}

void ChangeDirectory(char* currentDir, std::vector<std::string> command)						//3
{
	

}

void AddNewFile()							//4
{
	puts("\n\tFile added\n");
}

void AddNewDir(char* currentDir, std::vector<std::string> command)							//5
{
	const char* path = command[1].c_str();
	//path = strtok(command, " \t");
	AddDirectory(currentDir, path);
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

int  ExecuteCommand(char* currentDir)
{
	//char command[100];
	/*scanf("%s", command);*/
	
	std::string command;
	std::getline(std::cin, command, '\n');
	std::vector<std::string> argv = split(command, ' ');
	switch(GetCommandNum(argv[0]))
		{
			case 0:
				PrintHelp();
				break;
			case 1:
				ShowDirectory(currentDir);
				break;
			case 2:
				ChangeDirectory(currentDir, argv);
				break;
			case 3:
				AddNewFile();
				break;
			case 4:
				AddNewDir(currentDir, argv);
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
				return 1;
		}
	return 0;
}