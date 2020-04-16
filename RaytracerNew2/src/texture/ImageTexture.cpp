#include "ImageTexture.h"

#include "io/ImageLoader.h"
#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"
#include "core/Math.h"

ImageTexture::ImageTexture(const Parameters& params)
:Texture(params)
{
	std::string path = params.getString("path", "");
	myInvertY = params.getBool("invertY", false);
	myInvertX = params.getBool("invertX", false);
	myScale = std::abs(params.getReal("scale", (real)1.));
	myBoundaryCondition = parseBoundaryCondition(params.getString("boundaryCondition", "repeat"));
	real gamma = params.getReal("gamma", 0);
	ImageLoader::Channel channel = ImageLoader::channelToInt(params.getString("channel", "all"));
	myArray = ImageLoader::load(path, channel, gamma, 1.);

	init();
	ILogger::log() << "Texture min: " << getMin() << "Texture max: " << getMax() << "Texture average: " << getAverage();
}


ImageTexture::~ImageTexture()
{
}

Color ImageTexture::eval(const Point2d & uv)
{
	real x, y;
	if (!myInvertY) {
		y = uv.y() * myArray->getHeight();
	}
	else {
		//y = (1.f - uv.y()) * myArray.getHeight();
		y = (int)uv.y() + (math::sign(uv.y()) * 1.f - (uv.y() - (int)uv.y())) * myArray->getHeight();
	}

	if (!myInvertX) {
		x = uv.x() * (myArray->getWidth() - real(1.));
	} else {
		//x = (1.f - uv.x()) * (myArray.getWidth() - 1.);
		x = (int)uv.x() + (math::sign(uv.x()) * 1.f - (uv.x() - (int)uv.x())) * myArray->getWidth();
	}

	x *= myScale;
	y *= myScale;

	if (x >= myArray->getWidth())
		x = (int)x % myArray->getWidth() + (x - (int)x); //modulo + decimal part
	else if (x < 0)
		x = myArray->getWidth() - std::abs((int)x) % myArray->getWidth() + (x - (int)x);
	if (y >= myArray->getHeight())
		y = (int)y % myArray->getHeight() + (y  - (int)y);
	else if (y < 0)
		y = myArray->getHeight() - std::abs((int)y) % myArray->getHeight() + (y - (int)y);

#if NB_SPECTRUM_SAMPLES == 3
	//if (x >= myArray.getWidth() || x < 0 || y >= myArray.getHeight() || y < 0) {
	//	double tmp = uv.x() * (myArray.getWidth() - 1.);
	//	std::cout << x << " " << " " << y << " " << tmp
	//		<< " " << (myArray.getWidth() - 1) - std::abs((int)tmp) % myArray.getWidth() << " " << (tmp - (int)tmp) << std::endl;
	//	//y = (1.f - uv.y()) * myArray.getHeight();
	//	int val = myArray.getHeight() - std::abs((int)y) % myArray.getHeight();
	//	double dec = (1. - std::abs((y - (int)y)));
	//} else
	//return myArray((int)x, (int)y);
	return math::interp2(Point2d(x, y), *myArray);
#else
		Color3 res = math::interp2(Point2d(x, y), myArray);
		return Color::fromRGB(res(0), res(1), res(2));
#endif
	//return myArray(x, y);
}

void ImageTexture::init()
{
	Color res((real)0.);
	for (unsigned int i = 0; i < myArray->getWidth(); i++)
		for (unsigned int j = 0; j < myArray->getHeight(); j++)
			res += (*myArray)(i, j);

	res /= myArray->getWidth() * myArray->getHeight();
	myAverage = res;

	res = Color(+std::numeric_limits<real>::infinity());
	for (unsigned int i = 0; i < myArray->getWidth(); i++)
		for (unsigned int j = 0; j < myArray->getHeight(); j++)
			for (int k = 0; k < Color::NB_SAMPLES; k++)
				res(k) = std::min(res(k), (*myArray)(i, j)(k));

	myMin = res;

	res = Color(-std::numeric_limits<real>::infinity());
	for (unsigned int i = 0; i < myArray->getWidth(); i++)
		for (unsigned int j = 0; j < myArray->getHeight(); j++)
			for (int k = 0; k < Color::NB_SAMPLES; k++)
				res(k) = std::max(res(k), ((*myArray)(i, j))(k));

	myMax = res;
}

Color ImageTexture::getAverage() const
{
	return myAverage;
}

Color ImageTexture::getMin() const
{
	return myMin;
}

Color ImageTexture::getMax() const
{
	return myMax;
}

RT_REGISTER_TYPE(ImageTexture, Texture)