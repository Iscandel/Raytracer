#pragma once

#include "Array2D.h"
#include "Color.h"
#include "Texture.h"

#include <map>

class ImageLoader
{
public:
	typedef std::map<std::string, Array2D<Color>> CacheMap;
public:
	ImageLoader();
	~ImageLoader();

	static void load(const std::string& path, Array2D<Color>& array);
	static bool isHDRFile(const std::string& path);

protected:
	static CacheMap myCache;
	//Array2D<Color> myArray;
};

