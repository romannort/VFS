#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>

#include "Activities.h"

int main ( int argc, char** argv)
{
	InitFS();
	char* currentDir = new char[100];
	currentDir = "/";

	while( true )
	{
		printf("%s >>", currentDir);
		if (ExecuteCommand(currentDir))
		{
			break;
		}
	}
	return 0;
}