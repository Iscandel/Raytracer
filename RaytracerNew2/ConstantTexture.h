#pragma once
#include "Texture.h"

class Parameters;

class ConstantTexture :
	public Texture
{
public:
	ConstantTexture(const Parameters&);
	ConstantTexture(const Color& color);
	~ConstantTexture();

	Color eval(const Point2d&) override
	{
		return myValue;
	}

protected:
	Color myValue;
};

