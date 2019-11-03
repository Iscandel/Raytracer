#include "ConstantTexture.h"

#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"

ConstantTexture::ConstantTexture(const Parameters& params)
:Texture(params)
{
	myValue = params.getColor("albedo", Color(0.5));
}

ConstantTexture::ConstantTexture(const Color& color)
:Texture(Parameters())
{
	myValue = color;
}

ConstantTexture::ConstantTexture(real value)
:Texture(Parameters())
{
	myValue = Color(value);
}


ConstantTexture::~ConstantTexture()
{
}

RT_REGISTER_TYPE(ConstantTexture, Texture)