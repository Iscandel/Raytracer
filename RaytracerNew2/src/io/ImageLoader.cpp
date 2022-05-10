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

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <SFML/Graphics.hpp>

#include "core/Parameters.h"
#include "tools/Tools.h"


ImageLoader::CacheMap ImageLoader::myCache;

ImageLoader::ImageLoader()
{
}


ImageLoader::~ImageLoader()
{
}

std::shared_ptr<Array2D<Color3>> ImageLoader::load(const std::string & _path, Channel channel, real gammaFrom, real gammaDest)
{
	std::string path = getGlobalFileSystem().resolve(_path).string();
	std::size_t key = hash(path, channel);
	CacheMap::iterator it = myCache.find(key);
	if (it != myCache.end())
	{
		return it->second;
	}

	std::shared_ptr<Array2D<Color3>> array(new Array2D<Color3>);

	if(isHDRFile(path)) 
	{
		if (gammaFrom == 0)
			gammaFrom = (real)1;

		if (Imf::isOpenExrFile(path.c_str()))
		{
			Imf::RgbaInputFile file(path.c_str());
			Imath::Box2i dataWindow = file.dataWindow();
			int width = dataWindow.max.x - dataWindow.min.x + 1;
			int height = dataWindow.max.y - dataWindow.min.y + 1;
			Imf::Array2D<Imf::Rgba> pixels;
			pixels.resizeErase(height, width);
			file.setFrameBuffer(&pixels[0][0] - dataWindow.min.x - dataWindow.min.y * width, 1, width);
			file.readPixels(dataWindow.min.y, dataWindow.max.y);
			array->setSize(width, height);

			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					Imf::Rgba& rgba = pixels[j][i];
					real invGammaDest = gammaDest == (real)1. ? (real)1. : real(1. / gammaDest);
					Color3 pixel;

					pixel = setPixel(gammaFrom, invGammaDest, channel, (real)rgba.r, (real)rgba.g, (real)rgba.b, (real)rgba.a);

					(*array)(i, j) = pixel;//Color3::fromRGB(rgba.r, rgba.g, rgba.b);//Color(rgba.r, rgba.g, rgba.b);
				}
			}
		}
		else
		{
			int x, y, n;
			float *data = stbi_loadf(path.c_str(), &x, &y, &n, 0);
			if (n < 3 || n > 4)
				ILogger::log(ILogger::ERRORS) << "HDR image " << path << "has " << n << " components and should have 3 or 4\n";

			array->setSize(x, y);

			for (int i = 0; i < x; i++)
			{
				for (int j = 0; j < y; j++)
				{
					int idx = n * (j * x + i);
					float r = data[idx];
					float g = data[idx + 1];
					float b = data[idx + 2];
					float a = n == 4 ? data[idx + 3] : 0;
					real invGammaDest = gammaDest == (real)1. ? (real)1. : real(1. / gammaDest);
					Color3 pixel;

					pixel = setPixel(gammaFrom, invGammaDest, channel, r, g, b, a);

					(*array)(i, j) = pixel;//Color3::fromRGB(rgba.r, rgba.g, rgba.b);//Color(rgba.r, rgba.g, rgba.b);
				}
			}

			stbi_image_free(data);
		}
		//myCache[key] = array;
	}
	else
	{
		if (gammaFrom == 0)
			gammaFrom = (real)-1;

		sf::Image im;
		if (!im.loadFromFile(path))
			ILogger::log(ILogger::ERRORS) << "File " << path << " not found.\n";
		
		array->setSize(im.getSize().x, im.getSize().y);
		for (unsigned int y = 0; y < im.getSize().y; y++)
		{
			for (unsigned int x = 0; x < im.getSize().x; x++)
			{
				sf::Color col = im.getPixel(x, y);

				real invGammaDest = gammaDest == 1. ? (real)1. : (real)1. / gammaDest;
				Color3 pixel;
				real r = col.r / (real)255.; real g = col.g / (real)255.; real b = col.b / (real)255.; real a = col.a / (real)255.;
	
				pixel = setPixel(gammaFrom, invGammaDest, channel, r, g, b, a);

				(*array)(x, y) = pixel;
			}
		}

		
	}

	myCache[key] = array;
	return array;
}

bool ImageLoader::isHDRFile(const std::string & path)
{
	//std::ifstream f(path, std::ios_base::binary);
	//char b[4];
	//f.read(b, sizeof(b));

	//return !!f && b[0] == 0x76 && b[1] == 0x2f && b[2] == 0x31 && b[3] == 0x01;

	return Imf::isOpenExrFile(path.c_str()) || isRadianceFile(path);
}

bool ImageLoader::isRadianceFile(const std::string& path)
{
	std::ifstream f(path, std::ios_base::binary);
	if (!f)
		return false;

	char str[10];
	f.read(str, sizeof(str));
	if (memcmp(str, "#?RADIANCE", 10)) {
		f.close();
		return false;
	}

	f.close();
	return true;
}

ImageLoader::Channel ImageLoader::channelToInt(const std::string& channel)
{
	if (tools::lowerCase(channel) == "all")
		return ALL;
	if (tools::lowerCase(channel) == "r")
		return R;
	if (tools::lowerCase(channel) == "g")
		return G;
	if (tools::lowerCase(channel) == "b")
		return B;
	if (tools::lowerCase(channel) == "a")
		return A;
}

Color3 ImageLoader::setPixel(real gammaFrom, real invGammaDest, Channel channel, real r, real g, real b, real a)
{
	Color3 pixel;

	if (gammaFrom != 1 || invGammaDest != 1)
	{
		if (channel == A)
			pixel = spaceColorCorrection(gammaFrom, invGammaDest, a, a, a);
		else if (channel == R)
			pixel = spaceColorCorrection(gammaFrom, invGammaDest, r, r, r);
		else if (channel == G)
			pixel = spaceColorCorrection(gammaFrom, invGammaDest, g, g, g);
		else if (channel == B)
			pixel = spaceColorCorrection(gammaFrom, invGammaDest, b, b, b);
		else
			pixel = spaceColorCorrection(gammaFrom, invGammaDest, r, g, b);
	}
	else
	{
		if (channel == A)
			pixel = Color3::fromRGB(a, a, a);
		else if (channel == R)
			pixel = Color3::fromRGB(r, r, r);
		else if (channel == G)
			pixel = Color3::fromRGB(g, g, g);
		else if (channel == B)
			pixel = Color3::fromRGB(b, b, b);
		else
			pixel = Color3::fromRGB(r, g, b);
	}

	return pixel;
}

//Corrects the image applying gamma correction
Color3 ImageLoader::spaceColorCorrection(real gammaFrom, real invGammaDest, real r, real g, real b)
{
	//NB: We should iterate on each component of the spectrum and apply them the correction individually !!!!!!

	Color3 res;
	//-1 value means an sRGB curve. Reverse the sRGB applied to the texture and compute the linear RGB
	if (gammaFrom == -1)
		res = Color3::fromSRGB(r, g, b);
	else if(gammaFrom != 1) //if not sRGB, apply the source image gamma correction (2.2 is a common value
		res = Color3::fromRGB(std::pow(r, gammaFrom), std::pow(g, gammaFrom), std::pow(b, gammaFrom));

	//Get the linear RGB components
	res.toRGB(r, g, b);

	//if dest image is not intended to have a gamma 1 value, apply inverse gamma correction to fit dest image correction
	//
	if (invGammaDest == -1) {
		res.toSRGB(r, g, b);
		res = Color3::fromRGB(r, g, b); //actually it is a gamma correction that leads to srgb. Not sure if correct
	} else if(invGammaDest != 1)
		res = Color3::fromRGB(std::pow(r, invGammaDest), std::pow(g, invGammaDest), std::pow(b, invGammaDest));

	return res;	
}
