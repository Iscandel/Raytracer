#include "ScaleTexture.h"

#include "factory/ObjectFactoryManager.h"

ScaleTexture::ScaleTexture(const Parameters& params)
{
	myScale = params.getReal("scale", 1.);
	myTexture = params.getTexture("texture", Texture::ptr());

	if (myTexture == nullptr)
		ILogger::log() << "ScaleTexture: texture should not be null\n";
	ILogger::log() << "ScaleTexture min: " << getMin() << "ScaleTexture max: " << getMax() << "ScaleTexture average: " << getAverage();
}


ScaleTexture::~ScaleTexture()
{
}

Color ScaleTexture::eval(const Point2d& uv)
{
	return myTexture->eval(uv) * myScale;
}

Color ScaleTexture::getAverage() const
{
	return myTexture->getAverage() * myScale;
}

Color ScaleTexture::getMin() const
{
	return myTexture->getMin() * myScale;
}

Color ScaleTexture::getMax() const
{
	return myTexture->getMax() * myScale;
}

RT_REGISTER_TYPE(ScaleTexture, Texture)