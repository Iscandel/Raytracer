#include "ImageLoader.h"

#include <ImfArray.h>
#include <ImfInputFile.h>
#include <ImfRgbaFile.h>
#include <ImfRgba.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfTestFile.h>
#include <ImfStringAttribute.h>
#include <ImfVersion.h>
#include <ImfHeader.h>
#include <ImfIO.h>

#include <SFML/Graphics.hpp>


ImageLoader::CacheMap ImageLoader::myCache;

ImageLoader::ImageLoader()
{
}


ImageLoader::~ImageLoader()
{
}

void ImageLoader::load(const std::string & path, Array2D<Color>& array)
{
	CacheMap::iterator it = myCache.find(path);
	if (it != myCache.end())
	{
		array = it->second;
	}

	if (Imf_2_2::isOpenExrFile(path.c_str()))
	{
		Imf_2_2::RgbaInputFile file(path.c_str());
		Imath::Box2i displayWindow = file.displayWindow();
		//Imath::Box2i dw = file.dataWindow();
		int width = displayWindow.max.x - displayWindow.min.x + 1;
		int height = displayWindow.max.y - displayWindow.min.y + 1;
		Imf_2_2::Array2D<Imf_2_2::Rgba> pixels;
		pixels.resizeErase(height, width);
		file.setFrameBuffer(&pixels[0][0] - displayWindow.min.x - displayWindow.min.y * width, 1, width);
		file.readPixels(displayWindow.min.y, displayWindow.max.y);
		array.setSize(width, height);

		for (unsigned int i = 0; i < width; i++)
		{
			for (unsigned int j = 0; j < height; j++)
			{
				Imf_2_2::Rgba& rgba = pixels[j][i];
				array(i, j) = Color(rgba.r, rgba.g, rgba.b);	
			}
		}


		myCache[path] = array;
	}
	else
	{
		sf::Image im;
		if (!im.loadFromFile(path))
			ILogger::log(ILogger::ERRORS) << "File " << path << " not found.\n";
		
		array.setSize(im.getSize().x, im.getSize().y);
		for (unsigned int y = 0; y < im.getSize().y; y++)
		{
			for (unsigned int x = 0; x < im.getSize().x; x++)
			{
				sf::Color col = im.getPixel(x, y);
				array(x, y) = Color(col.r / 255., col.g / 255., col.b / 255.);
			}
		}

		myCache[path] = array;
	}
}

bool ImageLoader::isHDRFile(const std::string & path)
{
	std::ifstream f(path, std::ios_base::binary);
	char b[4];
	f.read(b, sizeof(b));

	return !!f && b[0] == 0x76 && b[1] == 0x2f && b[2] == 0x31 && b[3] == 0x01;
}
