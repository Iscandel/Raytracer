#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "Parameters.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class for reconstruction filter.
/// Filter functions are centered around 0.
///////////////////////////////////////////////////////////////////////////////
class ReconstructionFilter
{
public:
	typedef std::shared_ptr<ReconstructionFilter> ptr;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	ReconstructionFilter() { myRadiusX = myRadiusY = 0; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	ReconstructionFilter(const Parameters& params);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///////////////////////////////////////////////////////////////////////////
	ReconstructionFilter(real myRadiusX, real myRadiusY);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Destructor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~ReconstructionFilter(void);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Retrieves the correct filter function value at (x, y).
	///////////////////////////////////////////////////////////////////////////
	virtual real getValue(real x, real y) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief precomputes the set of values, from -radius to radius
	///////////////////////////////////////////////////////////////////////////
	void precompute();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Retrieves an approximation of the correct filter function value
	/// for the given point pair.
	///////////////////////////////////////////////////////////////////////////
	inline real operator() (real x, real y)
	{
		//check x and y range validity
		//int indexX = std::round(myResolution * x);
		int indexX = (int)((myResolution * std::abs(x)) / myRadiusX);// +0.5);
		indexX = std::min(indexX, myResolution - 1);
		int indexY = (int)((myResolution * std::abs(y)) / myRadiusY);// +0.5);
		indexY = std::min(indexY, myResolution - 1);
		return myPrecomputedValues[indexX + indexY * myResolution];
	}

	///////////////////////////////////////////////////////////////////////////
	///
	///////////////////////////////////////////////////////////////////////////
	real getRadiusX() {return myRadiusX;}
	real getRadiusY() {return myRadiusY;}

protected:
	real myRadiusX;
	real myRadiusY;

	int myResolution;

	std::vector<real> myPrecomputedValues;
};

