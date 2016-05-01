#pragma once
#include "Texture.h"
class PerlinNoiseTexture :
	public Texture
{
public:
	PerlinNoiseTexture();
	~PerlinNoiseTexture();

	Color eval(const Point2d& uv) override;
};

