#pragma once
#include <string>
#include <filesystem>
class Filesystem
{
public:
	typedef std::filesystem::path path;
	typedef std::vector<path>::iterator iterator;
	

public:
	Filesystem();
	~Filesystem();

	iterator begin() { return mySearchPaths.begin(); }
	iterator end() { return mySearchPaths.end(); }

	void addSearchPath(const std::string& path)
	{
		mySearchPaths.push_back(path);
	}

	void addSearchPath(const path& _path)
	{
		mySearchPaths.push_back(_path);
	}

	void removeSearchPath(iterator it)
	{
		mySearchPaths.erase(it);
	}

	Filesystem::path resolve(const Filesystem::path& toResolve)
	{
		for (path& s : mySearchPaths)
		{
			path fullPath = s / toResolve;
			if (std::filesystem::exists(fullPath))
				return fullPath;
		}

		return toResolve;
	}

protected:
	
	std::vector<path> mySearchPaths;
};

