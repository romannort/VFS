#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>

#include "Activities.h"



void main ( int argc, char** argv)
{
	InitFS();
	char* currentDir = new char[100];
	currentDir = "/";


	while( true )
	{
		
		printf("%s >>", currentDir);
		ExecuteCommand(currentDir);
	}
}