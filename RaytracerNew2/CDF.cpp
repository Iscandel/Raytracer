#include "CDF.h"



CDF::CDF()
:mySum(0.)
, myIsNormalized(false)
{
	myCDF.push_back(0.);
}


CDF::~CDF()
{
}
