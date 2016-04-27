#pragma once
#include "Texture.h"
class CheckerTexture :
	public Texture
{
public:
	CheckerTexture();

	~CheckerTexture();

	Color eval(const Point2d& uv) override
	{
		//std::cout << uv.x() << " " << uv.y() << std::endl;
		bool firstColor = false;
		int x = uv.x() / mySquareSize;
		if (x % 2 == 0)
			firstColor = true;
		else
			firstColor = false;

		int y = uv.y() / mySquareSize;
		if (y % 2 == 0)
		{
			if (firstColor)
				return myFirstColor;
			return mySecondColor;
		}
		else
		{
			return firstColor ? mySecondColor : myFirstColor;
		}
			
	}

protected:
	double mySquareSize;
	Color myFirstColor;
	Color mySecondColor;
};

