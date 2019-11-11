#include "Filesystem.h"



Filesystem::Filesystem()
{
	mySearchPaths.push_back(std::experimental::filesystem::current_path());
}


Filesystem::~Filesystem()
{
}
