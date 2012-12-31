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

void ShowDirectory(std::string currentDir)						//2
{
	std::vector<std::string> lst = ShowDirList(currentDir);
	printf("%d entries\n", lst.size());
	for ( int i = 0; i < lst.size(); ++i)
	{
		std::cout << lst[i] << std::endl;
	}
}

std::string AbsolutePath(std::string curDir, std::string path)
{
	if(path[0] == '/')
		return path;
	else
		if(curDir == "/")
			return curDir + path;
		else
			return curDir + '/' + path;
}

std::string ChangeDirectory(std::string curDir, std::string path)						//3
{
	Directory dir;
	Inode dirInode;
	
	path = AbsolutePath(curDir, path);
	if( GetDirByName(path, dir, dirInode) == -1)
		return curDir;
	else
		return path;
}

void AddNewFile()							//4
{
	puts("\n\tFile added\n");
}

void AddNewDir(std::string currentDir, std::vector<std::string> command)							//5
{
	const char* path = command[1].c_str();
	AddDirectory(currentDir, path);
}

void Remove(std::string currentDir, std::vector<std::string> command)
{
	RemoveDir(currentDir, command[1]);
}

void Copy()									//7
{
	puts("\n\tCopying done\n");
}

std::string GetParent(std::string path)
{
	size_t found;
	found = path.find_last_of("/");
	if(found == 0)
		return "/";
	return path.substr(0,found);
}

std::string GetLastFromPath(std::string path)
{
	size_t found;
	found = path.find_last_of("/");
	return path.substr(found+1);
}

void Move(std::string currentDir, std::vector<std::string> command)									//8
{
	Directory dir;
	Inode dirInode;
	std::string source = AbsolutePath(currentDir, command[1].c_str());
	std::string dest = AbsolutePath(currentDir, command[2].c_str());
	
	if(GetDirByName(source, dir, dirInode) == -1  || 
	   GetDirByName(source, dir, dirInode) == -1)
	   return;

	MoveHadler(GetParent(source), GetLastFromPath(source), dest);
//	puts("\n\tMoving done\n");
}

void Rename()								//9
{
	puts("\n\tRenaming done\n");
}

void NoSuchCommand(std::string command)
{
	std::cout << "'" << command <<"' is not recognized as internal command. Use 'help' to take command list\n";
}

void ExecuteCommand()
{
	//char command[100];
	/*scanf("%s", command);*/
	
	std::string currentDir = "/";
	std::string command;
	std::vector<std::string> argv;
	while(true)
	{
		std::cout << "\n" << currentDir << "# ";
		std::getline(std::cin, command, '\n');
		argv = split(command, ' ');
		switch(GetCommandNum(argv[0]))
			{
				case 0:
					PrintHelp();
					break;
				case 1:
					ShowDirectory(currentDir);
					break;
				case 2:
					currentDir = ChangeDirectory(currentDir, argv[1]);
					break;
				case 3:
					AddNewFile();
					break;
				case 4:
					AddNewDir(currentDir, argv);
					break;
				case 5:
					Remove(currentDir, argv);
					break;
				case 6:
					Copy();
					break;
				case 7:
					Move(currentDir, argv);
					break;
				case 8:
					Rename();
					break;
				case 9:
					return;
				default:
					if(argv[0] != "")
						NoSuchCommand(argv[0]);
					break;
			}
	}
}