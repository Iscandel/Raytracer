#pragma once
#include "Texture.h"
#include "Array2D.h"

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
};

