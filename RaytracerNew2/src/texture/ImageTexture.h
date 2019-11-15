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
	Color getAverage() const override;
	Color getMin() const override;
	Color getMax() const override;

protected:
	void init();

protected:
	std::shared_ptr<Array2D<Color3>> myArray;
	bool myInvertY;
	bool myInvertX;
	real myScale; //for texture repetition

	Color myMin;
	Color myAverage;
	Color myMax;
};

