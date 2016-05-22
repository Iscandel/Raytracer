#include "Screen.h"

#include "Logger.h"
#include "ReconstructionFilter.h"


//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Pixel::applyColor() 
{
	if(myWeight != 0.)
	{ 
		myColor /= myWeight; 
		myWeight = 0;
	} 

	//myColor.validate();
	//myColor *= 255;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Screen::Screen(int sizeX, int sizeY, int minX, int minY, ReconstructionFilter::ptr filter)
:mySizeX(sizeX)
,mySizeY(sizeY)
,myMinX(minX)
,myMinY(minY)
,myOverlapX(0)
,myOverlapY(0)
{
	if (filter)
		setFilter(filter);
	else
		myPixels.setSize(sizeX, sizeY);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Screen::~Screen(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Screen::addSample(double xx, double yy, const Color& value)
{	
	if (std::isnan(value.r) || std::isnan(value.g) || std::isnan(value.b))
	{
		ILogger::log() << "NaN value\n";
	}

	//Define the pixel center in the center and not upper left corner, 
	//adjust if to chunk coordinates and consider filter radius
	xx = xx - 0.5 - (myMinX - myOverlapX);
	yy = yy - 0.5 - (myMinY - myOverlapY);


	int startX = (int) std::ceil(xx - myFilter->getRadiusX());
	int endX = (int) std::floor(xx + myFilter->getRadiusX());
	int startY = (int) std::ceil(yy - myFilter->getRadiusY());
	int endY = (int) std::floor(yy + myFilter->getRadiusY());

	startX = std::max(0, startX);
	endX = std::min((int)myPixels.getWidth() - 1, endX);
	startY = std::max(0, startY);
	endY = std::min((int)myPixels.getHeight() - 1, endY);

	for (int x = startX; x <= endX; x++)
	{
		for (int y = startY; y <= endY; y++)
		{
			double weight = (*myFilter)(x - xx, y - yy);
			Pixel& pixel = myPixels(x, y);
			pixel.myColor += value * weight;
			pixel.myWeight += weight;
		}
	}

	//old
	//startX = std::max(myMinX, startX);
	//endX = std::min(myMinX + mySizeX - 1, endX);
	//startY = std::max(myMinY, startY);
	//endY = std::min(myMinY + mySizeY - 1, endY);

	//std::cout << startX << " " << endX << " " << xx << " " << startY << " " << yy << " " << endY << std::endl;

	//for(unsigned int x = startX; x < endX + 1; x++)
	//{
	//	for(unsigned int y = startY; y < endY + 1; y++)
	//	{
	//		double weight = (*myFilter)(x - xx, y - yy);
	//		Pixel& pixel = myPixels(x, y);
	//		pixel.myColor += value * weight;
	//		pixel.myWeight += weight;
	//	}
	//}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Screen::setFilter(ReconstructionFilter::ptr filter)
{
	myFilter = filter;

	myOverlapX = (int) std::ceil(myFilter->getRadiusX() - 0.5);
	myOverlapY = (int) std::ceil(myFilter->getRadiusY() - 0.5);

	myPixels.setSize(mySizeX + 2 * myOverlapX, mySizeY + 2 * myOverlapY);
	//Precompute filter values
	//myFilter->precompute();
}

void Screen::postProcessColor()
{
	for(int x = 0; x < mySizeX; x++)
	{
		for(int y = 0; y < mySizeY; y++)
		{
			myPixels(x, y).applyColor();
		}
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Screen> Screen::createSubScreen(int sizeX, int sizeY, int minX, int minY)
{
	std::unique_ptr<Screen> subScreen(new Screen(sizeX, sizeY, minX, minY));
	subScreen->setFilter(myFilter);

	return subScreen;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Screen::merge(Screen& subScreen)
{
	int offsetX = subScreen.getMinX() - myMinX + myOverlapX - subScreen.getOverlapX();
	int offsetY = subScreen.getMinY() - myMinY + myOverlapY - subScreen.getOverlapY();

	int sizeX = subScreen.getSizeX() + 2 * subScreen.getOverlapX();
	int sizeY = subScreen.getSizeY() + 2 * subScreen.getOverlapY();

	std::lock_guard<std::mutex> lock(myMergeMutex);
	
	int tmpX = 0;
	for (int x = offsetX; x < offsetX + sizeX; x++)
	{
		int tmpY = 0;
		for (int y = offsetY; y < offsetY + sizeY; y++)
		{
			myPixels(x, y).myColor += subScreen.getPixel(tmpX, tmpY).myColor;
			myPixels(x, y).myWeight += subScreen.getPixel(tmpX, tmpY).myWeight;
			tmpY++;
		}
		tmpX++;
	}
}
