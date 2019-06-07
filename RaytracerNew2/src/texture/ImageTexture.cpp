#include "ImageTexture.h"

#include "io/ImageLoader.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"
#include "core/Math.h"

ImageTexture::ImageTexture(const Parameters& params)
{
	std::string path = params.getString("path", "");
	myInvertY = params.getBool("invertY", false);
	myInvertX = params.getBool("invertX", false);
	myScale = std::abs(params.getReal("scale", (real)1.));
	ImageLoader::load(path, myArray);

	ILogger::log() << "Texture min: " << getMin() << "Texture max: " << getMax() << "Texture average: " << getAverage();
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
	real x, y;
	if (!myInvertY) {
		y = uv.y() * myArray.getHeight();
	}
	else {
		y = (1.f - uv.y()) * myArray.getHeight();
	}

	if (!myInvertX) {
		x = uv.x() * myArray.getWidth();
	} else {
		x = (1.f - uv.x()) * myArray.getWidth();
	}

	x *= myScale;
	y *= myScale;

	if (x >= myArray.getWidth())
		x = (int)x % myArray.getWidth() + (x - (int)x); //modulo + decimal part
	if (y >= myArray.getHeight())
		y = (int)y % myArray.getHeight() + (y  - (int)y);

#if NB_SPECTRUM_SAMPLES == 3
		return math::interp2(Point2d(x, y), myArray);
#else
		Color3 res = math::interp2(Point2d(x, y), myArray);
		return Color::fromRGB(res(0), res(1), res(2));
#endif
	//return myArray(x, y);
}

Color ImageTexture::getAverage() const
{
	Color res((real)0.);
	for (int i = 0; i < myArray.getWidth(); i++)
		for (int j = 0; j < myArray.getHeight(); j++)
			res += myArray(i, j);

	res /= myArray.getWidth() * myArray.getHeight();
	return res;
}

Color ImageTexture::getMin() const
{
	Color res(+std::numeric_limits<real>::infinity());
	for (int i = 0; i < myArray.getWidth(); i++)
		for (int j = 0; j < myArray.getHeight(); j++)
			for (int k = 0; k < Color::NB_SAMPLES; k++)
				res(k) = std::min(res(k), myArray(i, j)(k));

	return res;
}

Color ImageTexture::getMax() const
{
	Color res(-std::numeric_limits<real>::infinity());
	for (int i = 0; i < myArray.getWidth(); i++)
		for (int j = 0; j < myArray.getHeight(); j++)
			for (int k = 0; k < Color::NB_SAMPLES; k++)
				res(k) = std::max(res(k), myArray(i, j)(k));

	return res;
}

RT_REGISTER_TYPE(ImageTexture, Texture)