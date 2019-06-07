#pragma once
#include "Texture.h"

class Parameters;

class ConstantTexture :
	public Texture
{
public:
	ConstantTexture(const Parameters&);
	ConstantTexture(real value);
	ConstantTexture(const Color& color);
	~ConstantTexture();

	Color eval(const Point2d&) override
	{
		return myValue;
	}

	Color getAverage() const override
	{
		return myValue;
	}

	Color getMin() const override
	{
		return myValue;
	}

	Color getMax() const override
	{
		return myValue;
	}

protected:
	Color myValue;
};

