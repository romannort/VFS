#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>

#include "Activities.h"



void main ( int argc, char** argv)
{
	InitFS();
	ExecuteCommand("/");
}