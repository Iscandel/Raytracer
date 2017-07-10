#include "ImageTexture.h"

#include "ImageLoader.h"
#include "ObjectFactoryManager.h"
#include "Parameters.h"
#include "Tools.h"

#include <SFML/Graphics.hpp>

ImageTexture::ImageTexture(const Parameters& params)
{
	std::string path = params.getString("path", "");
	myInvertY = params.getBool("invertY", false);
	myInvertX = params.getBool("invertX", false);

	ImageLoader::load(path, myArray);
	//sf::Image im;
	//im.loadFromFile("./busteAjax.png");
	//myArray.setSize(im.getSize().x, im.getSize().y);
	//for (unsigned int y = 0; y < im.getSize().y; y++)
	//{
	//	for (unsigned int x = 0; x <im.getSize().x; x++)
	//	{
	//		sf::Color col = im.getPixel(x, y);
	//		myArray(x, y) = Color(col.r / 255., col.g / 255., col.b / 255.);
	//	}
	//}
}


ImageTexture::~ImageTexture()
{
}

Color ImageTexture::eval(const Point2d & uv)
{
	double x, y;
	if (!myInvertY) {
		y = uv.y() * myArray.getHeight();
	}
	else {
		y = (1. - uv.y()) * myArray.getHeight();
	}

	if (!myInvertX) {
		x = uv.x() * myArray.getWidth();
	} else {
		x = (1. - uv.x()) * myArray.getWidth();
	}

	return tools::interp2(Point2d(x, y), myArray);
	//return myArray(x, y);
}

RT_REGISTER_TYPE(ImageTexture, Texture)