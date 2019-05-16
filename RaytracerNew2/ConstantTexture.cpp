#include "ConstantTexture.h"

#include "ObjectFactoryManager.h"
#include "Parameters.h"

ConstantTexture::ConstantTexture(const Parameters& params)
{
	myValue = params.getColor("albedo", Color(0.5));
}

ConstantTexture::ConstantTexture(const Color& color)
{
	myValue = color;
}

ConstantTexture::ConstantTexture(real value)
{
	myValue = Color(value);
}


ConstantTexture::~ConstantTexture()
{
}

RT_REGISTER_TYPE(ConstantTexture, Texture)