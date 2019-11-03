#include "TwoSides.h"

#include "factory/ObjectFactoryManager.h"

TwoSides::TwoSides(const Parameters& params)
:BSDF(params)
{
	myBSDF[0] = params.getBSDF("bsdf", BSDF::ptr());
	myBSDF[1] = params.getBSDF("bsdf2", BSDF::ptr());

	if (myBSDF[0] == nullptr)
		ILogger::log(ILogger::ERRORS) << "Two sides: bsdf is null\n";
	if (myBSDF[1] == nullptr)
		myBSDF[1] = myBSDF[0];
}


TwoSides::~TwoSides()
{
}

Color TwoSides::eval(const BSDFSamplingInfos& infos)
{
	BSDFSamplingInfos infos2(infos);

	if (DifferentialGeometry::cosTheta(infos2.wi) > (real)0.f)
	{
		return myBSDF[0]->eval(infos2);
	}
	else
	{
		infos2.wi.z() = -infos.wi.z();
		infos2.wo.z() = -infos.wo.z();
		return myBSDF[1]->eval(infos2);
	}
}
Color TwoSides::sample(BSDFSamplingInfos& infos, const Point2d& sample)
{
	if (DifferentialGeometry::cosTheta(infos.wi) > (real)0.f)
	{
		return myBSDF[0]->sample(infos, sample);
	}
	else
	{
		infos.wi.z() = -infos.wi.z();
		Color res = myBSDF[1]->sample(infos, sample);
		infos.wi.z() = -infos.wi.z();
		infos.wo.z() = -infos.wo.z();

		return res;
	}
}
real TwoSides::pdf(const BSDFSamplingInfos& infos)
{
	BSDFSamplingInfos infos2(infos);

	if (DifferentialGeometry::cosTheta(infos2.wi) > (real)0.f)
	{
		return myBSDF[0]->pdf(infos2);
	}
	else
	{
		infos2.wi.z() = -infos.wi.z();
		infos2.wo.z() = -infos.wo.z();
		return myBSDF[1]->pdf(infos2);
	}
}

RT_REGISTER_TYPE(TwoSides, BSDF)