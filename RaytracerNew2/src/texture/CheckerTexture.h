#pragma once
#include "Texture.h"

class Parameters;

class CheckerTexture :
	public Texture
{
public:
	CheckerTexture(const Parameters& params);

	~CheckerTexture();

	Color eval(const Point2d& uv) override
	{
		//std::cout << uv.x() << " " << uv.y() << std::endl;
		bool firstColor = false;
		int x = (int)(uv.x() / mySquareSize);
		if (x % 2 == 0)
			firstColor = true;
		else
			firstColor = false;

		int y = (int)(uv.y() / mySquareSize);
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

	Color getAverage() const override
	{
		return (myFirstColor + mySecondColor) / (real)2;
	}

	Color getMin() const override
	{
		Color res(+std::numeric_limits<real>::infinity());
		for (int i = 0; i < Color::NB_SAMPLES; i++)
		{
			res(i) = math::min(res(i), myFirstColor(i), mySecondColor(i));
		}

		return res;
	}
	Color getMax() const override
	{
		Color res(-std::numeric_limits<real>::infinity());
		for (int i = 0; i < Color::NB_SAMPLES; i++)
		{
			res(i) = math::max(res(i), myFirstColor(i), mySecondColor(i));
		}

		return res;
	}

protected:
	real mySquareSize;
	Color myFirstColor;
	Color mySecondColor;
};

