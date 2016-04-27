#pragma once

#include "Color.h"
#include "Geometry.h"

class Texture
{
public:
	Texture();
	~Texture();

	virtual Color eval(const Point2d&) = 0;
};

