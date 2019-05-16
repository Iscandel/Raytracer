#pragma once
#include "Texture.h"
#include "tools/Array2D.h"

class Parameters;

class ImageTexture :
	public Texture
{
public:
	ImageTexture(const Parameters& params);
	~ImageTexture();

	Color eval(const Point2d & uv);

protected:
	Array2D<Color> myArray;
	bool myInvertY;
	bool myInvertX;
	real myScale;
};

