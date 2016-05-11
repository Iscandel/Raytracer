#include "CheckerTexture.h"

#include "ObjectFactoryManager.h"
#include "Parameters.h"

CheckerTexture::CheckerTexture(const Parameters& params)
{
	mySquareSize = params.getDouble("squareSize", 0.2);
	myFirstColor = params.getColor("firstColor", Color(1.));
	mySecondColor = params.getColor("secondColor", Color(0.));
}


CheckerTexture::~CheckerTexture()
{
}

RT_REGISTER_TYPE(CheckerTexture, Texture)