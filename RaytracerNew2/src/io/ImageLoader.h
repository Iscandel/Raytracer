#pragma once

#include "tools/Array2D.h"
#include "core/Color.h"
#include "texture/Texture.h"

#include <map>

class ImageLoader
{
public:
	typedef std::map<std::string, Array2D<Color3>> CacheMap;
public:
	ImageLoader();
	~ImageLoader();

	static void load(const std::string& path, Array2D<Color3>& array);
	static bool isHDRFile(const std::string& path);

protected:
	static CacheMap myCache;
	//Array2D<Color> myArray;
};

