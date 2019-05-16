#include "CheckerTexture.h"

#include "factory/ObjectFactoryManager.h"
#include "core/Parameters.h"

CheckerTexture::CheckerTexture(const Parameters& params)
{
	mySquareSize = params.getReal("squareSize", 0.2f);
	myFirstColor = params.getColor("firstColor", Color(1.));
	mySecondColor = params.getColor("secondColor", Color(0.));
}


CheckerTexture::~CheckerTexture()
{
}

RT_REGISTER_TYPE(CheckerTexture, Texture)