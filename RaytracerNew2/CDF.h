#pragma once

#include <algorithm>
#include <vector>

class CDF
{
public:
	CDF();
	~CDF();

	void add(double pdf)
	{
		mySum += pdf;
		myCDF.push_back(mySum);//myCDF.back() + pdf);
	}

	bool isNormalized() const { return myIsNormalized; }

	void normalize()
	{
		if (mySum != 0.)
		{
			double factor = 1. / mySum;
			for (unsigned int i = 1; i < myCDF.size(); i++)
			{
				myCDF[i] *= factor;
			}
			myCDF.back() = 1.;
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

	int sample(double sample) const
	{
		std::vector<double>::const_iterator it = std::lower_bound(myCDF.cbegin(), myCDF.cend(), sample);
		int index = std::max(0, it - myCDF.begin() - 1);
		//Last entry cannot be reached (1.)
		return std::min(index, (int) myCDF.size() - 2); 
	}

	int sample(double samp, double& pdf) const
	{
		int index = sample(samp);
		pdf = getNormalizedValue(index);

		return index;
	}

	double getNormalizedValue(int index) const
	{
		return myCDF[index + 1] - myCDF[index];
	}

	double getValue(int index) const
	{
		return (myCDF[index + 1] - myCDF[index]) * mySum;
	}

	//From Nori
	int sampleAndReuse(double &sampleVal) const {
		int index = sample(sampleVal);
		sampleVal = (sampleVal - myCDF[index]) / (myCDF[index + 1] - myCDF[index]);
		return index;
	}

	int sampleAndReuse(double &sampleVal, double& pdf) const {
		int index = sampleAndReuse(sampleVal);
		pdf = getNormalizedValue(index);
		return index;
	}

	double getSum() const { return mySum; }

	//first entry is a 0. So size - 1
	int getSize() const { return myCDF.size() - 1; }

protected:
	std::vector<double> myCDF;
	double mySum;
	bool myIsNormalized;
};

