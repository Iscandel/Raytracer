#include "ReconstructionFilter.h"


ReconstructionFilter::ReconstructionFilter(real radiusX, real radiusY)
:myResolution(32)
,myRadiusX(radiusX)
,myRadiusY(radiusY)
{
	//precompute();
}

ReconstructionFilter::ReconstructionFilter(const Parameters& params)
:myResolution(32)
{
	myRadiusX = params.getReal("radiusX", 0.5);
	myRadiusY = params.getReal("radiusY", 0.5);

	//precompute();
}


ReconstructionFilter::~ReconstructionFilter(void)
{
}

void ReconstructionFilter::precompute()
{
	myPrecomputedValues.resize(myResolution * myResolution);

	for(int x = 0; x < myResolution; x++)
	{
		for(int y = 0; y < myResolution; y++)
		{
			myPrecomputedValues[x + y * myResolution] = 
				getValue(x * myRadiusX / myResolution, 
						 y * myRadiusY / myResolution);
		}
	}
}
