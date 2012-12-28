char* Commands[10] = {"help", "ls", "cd", "mkfile", "mkdir", "rm", "cp", "mv", "rename?", "exit"};

int GetCommandNum()
{
	char command[25];

	scanf("%s", &command);
	for(int i = 0; i < 10; i++)
		if(!strcmp(Commands[i], command))
			return i;

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

void ShowDirectory()						//2
{
	puts("\n\tDirectory showed\n");
}

void ChangeDirectory()						//3
{
	puts("\n\tDirectory changed\n");
}

void AddNewFile()							//4
{
	puts("\n\tFile added\n");
}

void AddNewDir()							//5
{
	puts("\n\tDirectory added\n");
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