#include "Filesystem.h"



Filesystem::Filesystem()
{
	mySearchPaths.push_back(std::filesystem::current_path());
}


Filesystem::~Filesystem()
{
}
