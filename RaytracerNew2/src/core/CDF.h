#pragma once

#include <algorithm>
#include <vector>

#include "Geometry.h"

class CDF
{
public:
	CDF();
	~CDF();

	void add(real pdf)
	{
		mySum += pdf;
		myCDF.push_back(mySum);//myCDF.back() + pdf);
	}

	bool isNormalized() const { return myIsNormalized; }

	void normalize()
	{
		if (mySum != 0.)
		{
			real factor = 1.f / mySum;
			for (unsigned int i = 1; i < myCDF.size(); i++)
			{
				myCDF[i] *= factor;
			}
			myCDF.back() = 1.f;
			myIsNormalized = true;
		}
	}

	void clear()
	{
		myCDF.clear();
		myCDF.push_back(0.);
		myIsNormalized = false;
		mySum = 0.;
	}

	int sample(real sample) const
	{
		std::vector<real>::const_iterator it = std::lower_bound(myCDF.cbegin(), myCDF.cend(), sample);
		int index = std::max(0, (int)(it - myCDF.begin() - 1));
		//Last entry cannot be reached (1.)
		return std::min(index, (int) myCDF.size() - 2); 
	}

	int sample(real samp, real& pdf) const
	{
		int index = sample(samp);
		pdf = getNormalizedValue(index);

		return index;
	}

	real getNormalizedValue(int index) const
	{
		return myCDF[index + 1] - myCDF[index];
	}

	real getValue(int index) const
	{
		return (myCDF[index + 1] - myCDF[index]) * mySum;
	}

	//From Nori
	int sampleAndReuse(real &sampleVal) const {
		int index = sample(sampleVal);
		sampleVal = (sampleVal - myCDF[index]) / (myCDF[index + 1] - myCDF[index]);
		return index;
	}

	int sampleAndReuse(real &sampleVal, real& pdf) const {
		int index = sampleAndReuse(sampleVal);
		pdf = getNormalizedValue(index);
		return index;
	}

	real getSum() const { return mySum; }

	//first entry is a 0. So size - 1
	int getSize() const { return (int)(myCDF.size()) - 1; }

protected:
	std::vector<real> myCDF;
	real mySum;
	bool myIsNormalized;
};

