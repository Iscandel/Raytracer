#pragma once
#include "core/Parameters.h"
#include "Texture.h"

class ScaleTexture :
	public Texture
{
public:
	ScaleTexture(const Parameters&);
	~ScaleTexture();

	Color eval(const Point2d&) override;

	Color getAverage() const override;
	Color getMin() const override;
	Color getMax() const override;

protected:
	Texture::ptr myTexture;

	real myScale;
};

